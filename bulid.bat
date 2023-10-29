chcp 65001
if not exist build mkdir build
cd build
python3 ../configure.py --enable-optimize --plugin-name=antiflash --plugin-alias=antiflash --sdks=cs2 --targets=x86_64 --mms_path=C:\Users\Administrator\Desktop\metamodbulid\metamod-source --hl2sdk-root=C:\Users\Administrator\Desktop\cs2-sdk
ambuild
pause()