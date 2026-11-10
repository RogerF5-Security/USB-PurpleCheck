#include "../src/pc_i18n.h"
#include "../src/pc_settings.h"
#include "../src/probe_disk.h"
#include "../src/probe_status.h"
#include "../vendor/mass_storage_scsi.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static bool image_read(void *ctx, uint32_t lba, uint8_t out[512]) {
  if (lba >= PC_BLOCKS)
    return false;
  memcpy(out, (uint8_t *)ctx + lba * 512, 512);
  return true;
}
static unsigned writes_called = 0;
static bool count_write(void *ctx, uint32_t lba, uint16_t count, uint8_t *data,
                        uint32_t len) {
  (void)ctx;
  (void)lba;
  (void)count;
  (void)data;
  (void)len;
  writes_called++;
  return true;
}
static uint32_t blocks(void *c) {
  (void)c;
  return PC_BLOCKS;
}
static uint32_t be32(const uint8_t *p) {
  return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 |
         p[3];
}
static uint16_t le16(const uint8_t *p) {
  return (uint16_t)p[0] | (uint16_t)p[1] << 8;
}
static bool read_cb(void *c, uint32_t l, uint16_t n, uint8_t *out,
                    uint32_t *len, uint32_t cap) {
  (void)c;
  if (l >= PC_BLOCKS || n > PC_BLOCKS - l) {
    *len = 0;
    return false;
  }
  *len = cap / 512 * 512;
  memset(out, 0, *len);
  return true;
}
int main(int argc, char **argv) {
  assert(pc_crc32(0, (const uint8_t *)"123456789", 9) == 0xCBF43926u);
  PcSettings cfg;
  pc_settings_default(&cfg);
  assert(cfg.language == PcEnglish && cfg.probe_files &&
         pc_duration_ms(&cfg) == 60000);
  uint8_t config_bytes[PC_SETTINGS_BYTES];
  cfg.language = PcUkrainian;
  cfg.duration = 2;
  pc_settings_encode(&cfg, config_bytes);
  PcSettings decoded;
  assert(pc_settings_decode(&decoded, config_bytes, sizeof(config_bytes)));
  assert(decoded.language == PcUkrainian && pc_duration_ms(&decoded) == 120000);
  config_bytes[4] = 99;
  assert(!pc_settings_decode(&decoded, config_bytes, sizeof(config_bytes)));
  assert(decoded.language == PcEnglish);
  for (unsigned l = 0; l < PcLanguageCount; l++)
    for (unsigned t = 0; t < TxCount; t++)
      assert(pc_text(l, t)[0]);
  uint8_t *image = malloc(PC_BLOCKS * 512u);
  assert(image);
  const char *session = "PC0123456789ABCDEF";
  for (uint32_t i = 0; i < PC_BLOCKS; i++)
    pc_disk_sector(i, session, image + i * 512);
  uint32_t crc = 0;
  assert(pc_verify_return(image_read, image, session, &crc) == PcReturnNone);
  memcpy(image + 24 * 512, image + 16 * 512, PC_PAYLOAD_BYTES);
  assert(pc_verify_return(image_read, image, session, &crc) == PcReturnMatch);
  assert(crc == pc_crc32(0, image + 16 * 512, PC_PAYLOAD_BYTES));
  assert(pc_verify_return(image_read, image, "PCFFFFFFFFFFFFFFFF", &crc) ==
         PcReturnChanged);
  image[24 * 512 + 71] ^= 1;
  assert(pc_verify_return(image_read, image, session, &crc) == PcReturnChanged);
  image[24 * 512 + 71] ^= 1;
  image[13 * 512 + 96 + 28] = 0;
  image[13 * 512 + 96 + 29] = 0;
  assert(pc_verify_return(image_read, image, session, &crc) == PcReturnNone);
  free(image);
  assert(pc_observation(PcKeyboard, false, false, 0, 0, 0) == PcNoResponse);
  assert(pc_observation(PcKeyboard, true, false, 0, 0, 0) == PcHidConfigured);
  assert(pc_observation(PcKeyboard, true, true, 0, 0, 0) == PcHidSent);
  assert(pc_observation(PcDiskRw, false, false, 0, 0, 0) == PcNoResponse);
  assert(pc_observation(PcDiskRw, false, false, 1, 0, 0) == PcDiskRead);
  assert(pc_observation(PcDiskRw, false, false, 1, 1, 0) == PcDiskWritten);
  assert(pc_observation(PcDiskRo, false, false, 1, 0, 0) == PcDiskRead);
  assert(pc_observation(PcDiskRo, false, false, 1, 1, 0) == PcIoError);
  assert(pc_observation(PcDiskRw, false, false, 0, 0, 1) == PcIoError);
  const char *token = "PC0123456789ABCDEF";
  uint8_t b[512], fat[512], root[512];
  assert(pc_token_valid(token));
  assert(!pc_token_valid("PC0123456789ABCDEG"));
  assert(!pc_token_valid("PC0123"));
  pc_disk_sector(0, token, b);
  assert(b[510] == 0x55 && b[511] == 0xAA);
  assert(le16(b + 11) == 512);
  assert(b[13] == 1 && b[16] == 2);
  assert(le16(b + 22) == 6);
  assert(le16(b + 19) == PC_BLOCKS);
  pc_disk_sector(1, token, b);
  pc_disk_sector(7, token, fat);
  assert(!memcmp(b, fat, 512));
  // Cluster 2 is EOC; the shared nibble also encodes cluster 3 -> 4.
  assert(b[0] == 0xF8 && b[3] == 0xFF && (b[4] & 0x0F) == 0x0F);
  pc_disk_sector(PC_ROOT_START, token, root);
  assert(!memcmp(root + 32, "PROBE   TXT", 11));
  assert(le16(root + 58) == 2);
  pc_disk_sector(PC_DATA_START, token, b);
  assert(strstr((char *)b, token));
  assert(le16(root + 60) == strlen((char *)b));
  SCSISession s = {
      .fn = {.num_blocks = blocks, .read_only = true, .read = read_cb}};
  uint8_t cmd[16] = {0x2A};
  assert(!scsi_cmd_start(&s, cmd, 10));
  assert(s.sk == 7 && s.asc == 0x27);
  cmd[0] = 0x1A;
  assert(scsi_cmd_start(&s, cmd, 6));
  uint32_t n = 0;
  assert(scsi_cmd_tx_data(&s, b, &n, 512));
  assert(n == 4 && b[2] == 0x80);
  assert(scsi_cmd_end(&s));
  s.fn.read_only = false;
  assert(scsi_cmd_start(&s, cmd, 6));
  assert(scsi_cmd_tx_data(&s, b, &n, 512));
  assert(b[2] == 0);
  cmd[0] = 0x25;
  assert(scsi_cmd_start(&s, cmd, 10));
  assert(scsi_cmd_tx_data(&s, b, &n, 512));
  assert(be32(b) == PC_BLOCKS - 1 && be32(b + 4) == 512);
  cmd[0] = 0x23;
  assert(scsi_cmd_start(&s, cmd, 10));
  assert(scsi_cmd_tx_data(&s, b, &n, 512));
  assert(be32(b + 4) == PC_BLOCKS);
  assert(!scsi_cmd_start(&s, cmd, 0));
  assert(!scsi_cmd_start(&s, cmd, 17));
  cmd[0] = 0x28;
  assert(!scsi_cmd_start(&s, cmd, 5));
  cmd[0] = 0x2A;
  cmd[7] = 0;
  cmd[8] = 1;
  s.fn.write = count_write;
  assert(scsi_cmd_start(&s, cmd, 10));
  writes_called = 0;
  assert(!scsi_cmd_rx_data(&s, b, 511));
  assert(writes_called == 0);
  if (argc == 2) {
    FILE *f = fopen(argv[1], "wb");
    assert(f);
    for (unsigned i = 0; i < PC_BLOCKS; i++) {
      pc_disk_sector(i, token, b);
      assert(fwrite(b, 1, 512, f) == 512);
    }
    assert(!fclose(f));
  }
  puts("PASS: settings CRC, locales, synthetic return integrity, FAT12, "
       "observations, SCSI protection, capacities and CDB bounds");
  return 0;
}
