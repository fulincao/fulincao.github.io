
# cmake ndk 交叉编译

---

1. 从官网下载ndk

2. 生成独立的ndk工具链

    ```shell
    ## 生成arm64版本的
    ./build/tools/make_standalone_toolchain.sh \
    --arch arm64 \
    --install-dir=alone-sdk
    ```

3. cmake 配置

    ```cmake
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie")

    # 配置使用 NDK Standalone Toolchain 编译
    set(NDK_STANDALONE_TOOLCHAIN /home/cao/Android/Sdk/ndk/21.3.6528147/alone-sdk)
    # set(CMAKE_SYSTEM_NAME AndroidARM)
    # set(CMAKE_SYSTEM_VERSION 3)
    set(CMAKE_C_COMPILER ${NDK_STANDALONE_TOOLCHAIN}/bin/aarch64-linux-android-gcc)
    set(CMAKE_CXX_COMPILER ${NDK_STANDALONE_TOOLCHAIN}/bin/aarch64-linux-android-g++)
    # set(CMAKE_FIND_ROOT_PATH ${NDK_STANDALONE_TOOLCHAIN})

    set (CMAKE_ANDROID_ARCH_ABI arm64-v8a)

    # set (CMAKE_ANDROID_STANDALONE_TOOLCHAIN ~/ndk_toolchain)
    # 使用 NDK 提供的头文件
    add_definitions("--sysroot=${NDK_STANDALONE_TOOLCHAIN}/sysroot")
    ```

4. android引入so
    - 将生成的.so放到src/main/jniLibs/arm64-v8a/,并将.h复制到默认的src/main/cpp

    - 配置CMakeLists

    ```cmake
    add_library(dds SHARED IMPORTED)

    #设置so路劲  ${CMAKE_SOURCE_DIR}是CMakeLists.txt的路径   ${ANDROID_ABI} 标识cpu类型
    set_target_properties(dds PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libdds.so)
    set_property(TARGET dds PROPERTY IMPORTED_NO_SONAME 1)

    # 链接dds库
    target_link_libraries( # Specifies the target library.
        native-lib
        dds
        flow_stream
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

    ```

    - 配置app/build.gradle
  
    ```python

    ## defaultConfig下配置
    externalNativeBuild {
        cmake {
            cppFlags ""
            arguments "-DANDROID_STL=c++_shared"
        }
    }

    ndk {
        # 这里只配置 arm64-v8a,因为只生成了这个版本的库
        abiFilters 'arm64-v8a'
    }


    ## android 标签下配置
    splits {
        abi {
            enable true
            reset()
            include 'x86', 'x86_64', 'armeabi-v7a', 'arm64-v8a' //select ABIs to build APKs for
            universalApk true //generate an additional APK that contains all the ABIs
        }
    }
    # map for the version code
    project.ext.versionCodes = ['armeabi': 1, 'armeabi-v7a': 2, 'arm64-v8a': 3, 'mips': 5, 'mips64': 6, 'x86': 8, 'x86_64': 9]
    android.applicationVariants.all { variant ->
        // assign different version code for each output
        variant.outputs.each { output ->
            output.versionCodeOverride =
                    project.ext.versionCodes.get(output.getFilter(com.android.build.OutputFile.ABI), 0) * 1000000 + android.defaultConfig.versionCode
        }
    }

    ```

5. android使用
   - 使用默认生成native-lib.cpp include头文件,创建好暴露接口即可
   - 附数据对应 <https://blog.csdn.net/smilestone322/article/details/88607717>

---
