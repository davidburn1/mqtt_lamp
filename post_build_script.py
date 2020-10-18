Import("env", "projenv")
from shutil import copyfile

def after_build(source, target, env):
    print("after_build script")
    print("the target is ", str(target[0]))
    copyfile(str(target[0]), 'bin/firmware.bin')

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)

#build_tag = "v2.2"
#env.Replace(PROGNAME="firmware_%s" % build_tag)

