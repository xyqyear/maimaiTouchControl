// original author: HelloworldDk
// original file: https://github.com/HelloworldDk/dkmaiproj/blob/main/newmap-llvermtn202212271340/newmap-llvermtn202212271340.ino

#define SerialDevice Serial
#include "Adafruit_MPR121.h"
#include "config.h"

Adafruit_MPR121 mpr[4];
uint8_t packet[6];
uint8_t len = 0;
bool Conditioning = true;
uint32_t lastMillis = 0;

enum
{
  commandRSET = 0x45,  // E
  commandHALT = 0x4C,  // L
  commandSTAT = 0x41,  // A
  commandRatio = 0x72, // r
  commandSens = 0x6B,  // k
};

uint8_t DATA_READ_INTERVAL;

void setup()
{
  SerialDevice.begin(9600);
  SerialDevice.setTimeout(0);
  mpr[0].begin(0x5A, &Wire);
  mpr[1].begin(0x5B, &Wire);
  mpr[2].begin(0x5C, &Wire);
  mpr[3].begin(0x5D, &Wire);
  Wire.setClock(800000);

  // config conversion

  uint8_t SECOND_FILTER_ITERATION_SAMPLES;
  uint8_t ELECTRODE_SAMPLE_INTERVAL_MS;

  switch (SECOND_FILTER_ITERATIONS)
  {
  case 0:
    SECOND_FILTER_ITERATION_SAMPLES = 4;
    break;
  case 1:
    SECOND_FILTER_ITERATION_SAMPLES = 6;
    break;
  case 2:
    SECOND_FILTER_ITERATION_SAMPLES = 10;
    break;
  case 3:
    SECOND_FILTER_ITERATION_SAMPLES = 18;
    break;
  }

  ELECTRODE_SAMPLE_INTERVAL_MS = 1 << ELECTRODE_SAMPLE_INTERVAL;
  DATA_READ_INTERVAL = SECOND_FILTER_ITERATION_SAMPLES * ELECTRODE_SAMPLE_INTERVAL_MS;
}

void loop()
{
  // there will only be a new data point every this many milliseconds
  // so we don't need to check every loop
  uint16_t currentMillis = millis();
  if (currentMillis - lastMillis > DATA_READ_INTERVAL)
  {
    lastMillis = currentMillis;
    Recv();
    Conditioning ? void() : TouchSend();
  }
}

void MprSetup(Adafruit_MPR121 cap)
{
  // @datasheet page 19,
  // Write 0x80 with 0x63 asserts soft reset.
  // The soft reset does not effect the I2C module, but all others reset the same as POR.
  cap.writeRegister(MPR121_SOFTRESET, 0x63);

  delay(1);

  // @datasheet page 16
  // 2bit Cl, 2bit ELEPROX_EN, 4bit ELE_EN
  // ELE_EN being 0 means disbled
  cap.writeRegister(MPR121_ECR, 0x0);

  // @AN3891
  // those values control the baseline tracking to environment changes
  // suffix R and F means rising and falling
  // falling is the touch direction, rising is the release direction
  // MHD is the max half delta, the max difference between the baseline and the data
  //   if the difference is larger than this value, the baseline will be updated (NCL and FDL conditions must also be met)
  // NCL is the noise count limit, at least this many of changes must be larger than MHD to trigger a baseline update
  // FDL is the filter delay count limit, the baseline will be updated after this many samples
  //   if a touch is detected mid way, the update is cancelled
  // NHD is the noise half delta, the baseline will be changed this much when the above conditions are met
  // please note that the actual update delay is affected by the second stage filter
  cap.writeRegister(MPR121_MHDR, MHDR);
  cap.writeRegister(MPR121_NHDR, NHDR);
  cap.writeRegister(MPR121_NCLR, NCLR);
  cap.writeRegister(MPR121_FDLR, FDLR);
  cap.writeRegister(MPR121_MHDF, MHDF);
  cap.writeRegister(MPR121_NHDF, NHDF);
  cap.writeRegister(MPR121_NCLF, NCLF);
  cap.writeRegister(MPR121_FDLF, FDLF);

  // @AN3892 page 7
  // 1 bit unused, 3 bit DR, 1 bit unused, 3 bit DT
  // DR is the debounce count for release detection
  // DT is the debounce count for touch detection
  // for example, when DR is set to 0b010, the debounce count is 4
  //     which means there must be 4 consecutive samples's delta is less than the threshold to trigger a release
  // dk's value is 4 << 4 | 2, meaning DR is 4, DT is 2
  cap.writeRegister(MPR121_DEBOUNCE, RELEASE_DEBOUNCE << 4 | TOUCH_DEBOUNCE);

  // @AN3890 page 2
  // 2 bit FFI, 6 bit CDC
  // the default value is 10, meaning
  // First Filter Iterations is 0b00: 6 samples. toss the hightest and lowest average the rest
  // Charge Discharge Current is 10: 10uA, which will be overriden by autoconfig
  // the value set by dk is 16, but it will be overriden by autoconfig, so it doesn't matter
  cap.writeRegister(MPR121_CONFIG1, FIRST_FILTER_ITERATIONS << 6);

  // @AN3890 page 3
  // 3 bit CDT, 2 bit SFI, 3 bit ESI
  // the default value is 0b00011000, meaning
  // CDT is 0b000: invalid, dk set it to 0b001: 0.5us
  //     the value here is 2^(CDT-2) us
  // SFI is number of samples to take for second filter
  // ESI selects the period between samples for second filter
  // ! the touch response time is DT * SFI * ESI ms
  // ! and the release response time is DR * SFI * ESI ms
  // we can set SFI to 0b00 which is 4 samples, and ESI to 0b000, which is 1ms, if DR and DT is set to 4, the response time will be 16ms, approximately one frame
  cap.writeRegister(MPR121_CONFIG2, CHARGE_DISCHARGE_TIME << 5 | SECOND_FILTER_ITERATIONS << 3 | ELECTRODE_SAMPLE_INTERVAL);

  // @AN3889 page 9
  // 2 bit AFES, 2 bit RETRY, 2 bit BVA, 1 bit ARE, 1 bit ACE
  // AFES must match FFI setting in CONFIG1, which is 0b00: 6 samples
  // RETRY is 0b00, disabled, which is said to be not required in production systems.
  // BVA should be set to 0b10, which allows the baseline to be updated
  //     10 is used instead of 11 because this guarantees that the baseline will be lower than the data
  // ARE should be 1 to enable auto configuration
  // ACE should be 1 to enable auto reconfiguration
  //     reconfiguration will trigger any time the baseline drifts outside the range set by the USL and the LSL
  // in conclusion, this register should be set to 0b00001011, the same as described in AN3944 page 4
  cap.writeRegister(MPR121_AUTOCONFIG0, FIRST_FILTER_ITERATIONS << 6 | AUTO_CONFIG_RETRY << 4 | AUTO_CONFIG_BVA << 2 | AUTO_CONFIG_ARE << 1 | AUTO_CONFIG_ACE);

  // @datasheet page 15 and 18
  // 1bit SCTS, 4bit empty, 3bit other config that we don't care.
  // SCTS being 1 means individual electrode CDT(charge/discharge time) search is disabled
  // "Using only the global CDC and/or global CDT is acceptable where the capacitance values from all 13 channels are similar. If the electrode pattern, size, or even overlay and base material type changes from one channel to another, then using individual CDCx (and CDTx) will have a better result on sensing sensitivity as each electrode is charged up to a point closing to the supply voltage rail so that the highest sensing field is built for each channel."
  // so we should not disable SCTS.
  cap.writeRegister(MPR121_AUTOCONFIG1, SCTS << 7);

  // @AN3889 page 7 through 8
  cap.writeRegister(MPR121_UPLIMIT, USL);
  cap.writeRegister(MPR121_TARGETLIMIT, TL);
  cap.writeRegister(MPR121_LOWLIMIT, LSL);

  // @datasheet page 16
  // 2bit CL, 2bit ELEPROX_EN, 4bit ELE_EN
  // CL set to 11 means Baseline tracking enabled, initial baseline value is loaded with all 10 bits of the first electrode data value
  // ELEPROX_EN being 0 means proximity disabled
  // ELE_EN being a certain number means how many electrodes are enabled
  // the ELE_EN value set here is irrelevant because we will set it again in STAT command
  cap.writeRegister(MPR121_ECR, CALIBRATION_LOCK << 6 | 12);
}

void cmd_RSET()
{
  MprSetup(mpr[0]);
  MprSetup(mpr[1]);
  MprSetup(mpr[2]);
  MprSetup(mpr[3]);
}
void cmd_HALT()
{
  // Start Conditioning Mode
  mpr[0].writeRegister(MPR121_ECR, 0);
  mpr[1].writeRegister(MPR121_ECR, 0);
  mpr[2].writeRegister(MPR121_ECR, 0);
  mpr[3].writeRegister(MPR121_ECR, 0);
  Conditioning = true;
}

void cmd_Ratio()
{
  // Set Touch Panel Ratio
  SerialDevice.write('(');
  SerialDevice.write(packet + 1, 4);
  SerialDevice.write(')');
}

void cmd_Sens()
{
  // Set Touch Panel Sensitivity
  SerialDevice.write('(');
  SerialDevice.write(packet + 1, 4);
  SerialDevice.write(')');
}

void cmd_STAT()
{
  // End Conditioning Mode
  Conditioning = false;

  // setting individual electrode sensitivity
  for (uint8_t i = 0; i < 34; i++)
  {
    uint8_t mprid = touchmap[i].mprid;
    uint8_t portid = touchmap[i].portid;
    int8_t thresOffset = touchmap[i].thresOffset;
    mpr[mprid].writeRegister(MPR121_TOUCHTH_0 + portid * 2, THREASHOLD + thresOffset);
    mpr[mprid].writeRegister(MPR121_RELEASETH_0 + portid * 2, THREASHOLD - RELEASE_THREASHOLD_OFFSET + thresOffset);
  }

  // setting ECR to enable electrodes
  mpr[0].writeRegister(MPR121_ECR, CALIBRATION_LOCK << 6 | MPR_PADNUMS[0]);
  mpr[1].writeRegister(MPR121_ECR, CALIBRATION_LOCK << 6 | MPR_PADNUMS[1]);
  mpr[2].writeRegister(MPR121_ECR, CALIBRATION_LOCK << 6 | MPR_PADNUMS[2]);
  mpr[3].writeRegister(MPR121_ECR, CALIBRATION_LOCK << 6 | MPR_PADNUMS[3]);
}

void Recv()
{
  while (SerialDevice.available())
  {
    uint8_t r = SerialDevice.read();
    if (r == '{')
    {
      len = 0;
    }
    if (r == '}')
    {
      break;
    }
    packet[len++] = r;
  }
  if (len == 5)
  {
    switch (packet[3])
    {
    case commandRSET:
      cmd_RSET();
      break;
    case commandHALT:
      cmd_HALT();
      break;
    case commandRatio:
      cmd_Ratio();
      break;
    case commandSens:
      cmd_Sens();
      break;
    case commandSTAT:
      cmd_STAT();
      break;
    }
    len = 0;
    memset(packet, 0, 6);
  }
}

void TouchSend()
{
  // setting up the stage and reading the touched data
  uint64_t TouchData = 0;
  uint16_t mprTouchedData[] = {
      mpr[0].touched(),
      mpr[1].touched(),
      mpr[2].touched(),
      mpr[3].touched(),
  };

  // mapping the touched data to the correct order
  for (int8_t i = 33; i >= 0; i--)
  {
    TouchData <<= 1;
    TouchData = (TouchData | ((mprTouchedData[touchmap[i].mprid] >> touchmap[i].portid) & 1));
  }

  // sending the data
  uint8_t data[9];
  data[0] = '(';
  data[8] = ')';
  for (uint8_t i = 1; i < 8; i++)
  {
    data[i] = (byte)TouchData & B11111;
    TouchData >>= 5;
  }
  SerialDevice.write(data, 9);
}
