Import("env", "projenv")
from shutil import copyfile

def after_build_firmware(source, target, env):
    print("after_build firmware")
    print("the target is ", str(target[0]))
    copyfile(str(target[0]), 'bin/firmware.bin')


def after_build_littleFS(source, target, env):
    print("after_build littleFS")
    copyfile(str(target[0]), 'bin/littlefs.bin')
    #copyfile(".pio/build/d1_mini/littlefs.bin", 'bin/littlefs.bin')


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build_firmware)
env.AddPostAction(".pio/build/d1_mini/littlefs.bin", after_build_littleFS)

#build_tag = "v2.2"
#env.Replace(PROGNAME="firmware_%s" % build_tag)

