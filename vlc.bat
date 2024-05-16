
@echo off

REM Ensure the directory exists on the device
adb shell mkdir -p /data/local/tmp

REM Push the scrcpy server to the device
adb push scrcpy-server-v2.4 /data/local/tmp/scrcpy-server-manual.jar

REM Forward the port
adb forward tcp:1234 localabstract:scrcpy

REM Launch the scrcpy server with specified parameters
adb shell CLASSPATH=/data/local/tmp/scrcpy-server-manual.jar app_process / com.genymobile.scrcpy.Server 2.4 raw_stream=true tunnel_forward=true audio=false send_codec_meta=true send_frame_meta=true control=false cleanup=false


