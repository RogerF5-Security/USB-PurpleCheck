# Validation - 2026-11-10

## 0.3 development candidate

- Native C tests pass: settings defaults/CRC and invalid-value fallback, all translation entries, Ukrainian bitmap coverage, synthetic return matching/mismatch/session binding, FAT12 geometry, separated observations and SCSI write protection/capacity/bounds. Misaligned writes are rejected before calling the storage backend.
- Compiled and linked with Momentum MNTM-012 SDK, f7 API 87.1; APPCHK passed. This is not an official-release compatibility claim.
- Official-release CI and the new on-device UI checks are being prepared; no completion is claimed in this snapshot.
- Hardware acceptance remains pending for actual HID text reception, MSC mount/read/write/return, repeated profile switching, cancellation and timeout under active host I/O. Endpoint policy findings additionally require an independently documented baseline and enforcement evidence.

## 0.2 hardware observations retained for provenance

## Completed
- Compiled and linked with the installed Momentum MNTM-012 SDK, target f7, API 87.1. uFBT APPCHK passed.
- Uploaded over the existing FAP at /ext/apps/USB/usb_purplecheck.fap. qFlipper logs visibly reported Storage Write SUCCESS.
- Launched on the connected physical Flipper through qFlipper remote control. The display visibly showed PurpleCheck 0.2 and the three Spanish menu entries: Teclado USB, Disco lectura/escrit., Disco solo lectura.
- Native C tests passed for FAT12 geometry, exact token/ACK validation, SCSI write-protection, capacity responses and CDB length limits.
- Nine additional native observation assertions passed: no response, HID configured, HID marker submitted, disk read, disk write, read-only read, unexpected read-only write and I/O errors remain distinct. No observation state automatically claims an endpoint-policy block.

## Pending
- Actual HID input delivery, physical MSC mount/read/write, repeated profile switching, cancellation, SD persistence/reload and timeout behavior on hardware.
- Endpoints with independently documented allow/block policies and vendor enforcement evidence.

No live USB policy verdict is claimed. Starting a profile requires qFlipper/RPC to release the firmware USB lock. All operation after installation uses the physical Flipper; no companion is part of 0.2.

The earlier PowerShell receiver is not part of this standalone release. Its signing error is no longer in the user flow.
