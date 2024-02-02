set(OHOS_INCLUDE_PATHS 
    base/hiviewdfx/hilog_lite/interfaces/native/kits/hilog_lite 
    commonlibrary/utils_lite/include 
    base/hiviewdfx/hilog_lite/command 
    base/hiviewdfx/hilog_lite/frameworks/mini 
    kernel/liteos_m/kernel/include 
    kernel/liteos_m/utils 
    base/startup/bootstrap_lite/services/source 
    foundation/systemabilitymgr/samgr_lite/interfaces/kits/samgr 
    foundation/bundlemanager/bundle_framework_lite/interfaces/kits/bundle_lite 
    foundation/bundlemanager/bundle_framework_lite/utils/bundle_lite 
    third_party/bounds_checking_function/include 
    foundation/ability/ability_lite/interfaces/kits/want_lite
    commonlibrary/utils_lite/include
)

set(OHOS_LIBS "${ohos_lib_path}/libbegetutil.a \
${ohos_lib_path}/libbootstrap.a \
${ohos_lib_path}/libhal_file_static.a \
${ohos_lib_path}/libhal_sysparam.a \
${ohos_lib_path}/libhievent_lite_static.a \
${ohos_lib_path}/libhilog_lite_static.a \
${ohos_lib_path}/libhilog_static.a \
${ohos_lib_path}/libhiview_lite_static.a \
${ohos_lib_path}/libinit_log.a \
${ohos_lib_path}/libinit_utils.a \
${ohos_lib_path}/libnative_file.a \
${ohos_lib_path}/libparam_client_lite.a \
${ohos_lib_path}/libsamgr.a \
${ohos_lib_path}/libsamgr_adapter.a \
${ohos_lib_path}/libsamgr_source.a \
${ohos_lib_path}/libsec_static.a \
")
set(OHOS_FLAGS "-Wl,--whole-archive ${OHOS_LIBS} -Wl,--no-whole-archive")

######################################ohos demo###########################

set(target ohos_demo)
add_appimg_flash_example(${target} main.c demo.c)
include_directories(${INC_DIR})
include_directories(${INC_DIR}/system)

target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_OHOS)
relative_glob(srcs *.[h|c|cpp])
beautify_sdk_code(${target} ${srcs})


foreach(inc ${OHOS_INCLUDE_PATHS})
    include_directories(${ohos_inc_path}/${inc})
endforeach()
set_target_properties(${target} PROPERTIES LINK_FLAGS "${OHOS_FLAGS}")
if (BUILD_OHOS)
    add_dependencies(${target} sdk_ohos_libs)
endif()

######################################XTS start###########################
set(OHOS_LIBS_XTS "")
macro(add_target_ohos_xts)
    foreach(arg ${ARGN})
        set(OHOS_XTS_LIBS "${ohos_lib_path}/libhctest.a \
            ${ohos_lib_path}/libmodule_${arg}.a \
            ")
        set(target ohos_xts_${arg})
        add_appimg_flash_example(${target} ohos-main.c)
        include_directories(${INC_DIR})
        target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_OHOS)
        set(OHOS_FLAGS "-Wl,--whole-archive ${OHOS_LIBS} ${OHOS_LIBS_XTS} ${OHOS_XTS_LIBS} -Wl,--no-whole-archive")
        set_target_properties(${target} PROPERTIES LINK_FLAGS "${OHOS_FLAGS}")
        if (BUILD_OHOS)
            add_dependencies(${target} sdk_ohos_libs)
        endif()
    endforeach()
endmacro()

###########################################################################
#ActsBootstrapTest ActsDfxFuncTest ActsHieventLiteTest ActsUtilsFileTest
###########################################################################
add_target_ohos_xts(ActsBootstrapTest ActsDfxFuncTest ActsHieventLiteTest ActsUtilsFileTest)

###########################################################################
#ActsSamgrTest
###########################################################################
set(OHOS_LIBS_XTS "${ohos_lib_path}/libbroadcast.a")
add_target_ohos_xts(ActsSamgrTest)

###########################################################################
#ActsParameterTest
###########################################################################
set(OHOS_LIBS_XTS "${ohos_lib_path}/libinithook.a")
add_target_ohos_xts(ActsParameterTest)

###########################################################################
#ActsUpdaterFuncTest
###########################################################################
set(OHOS_LIBS_XTS "${ohos_lib_path}/libhal_update_static.a \
    ${ohos_lib_path}/libhota.a \
    ")
add_target_ohos_xts(ActsUpdaterFuncTest)

###########################################################################
#ActsHuksHalFunctionTest
###########################################################################
set(OHOS_LIBS_XTS "${ohos_lib_path}/libhuks_3.0_sdk.a \
    ${ohos_lib_path}/libhuks_test_common.a \
    ")
add_target_ohos_xts(ActsHuksHalFunctionTest)

###########################################################################
#ActsBundleMgrTest
###########################################################################
set(OHOS_LIBS_XTS "${ohos_lib_path}/libbundle.a \
    ${ohos_lib_path}/libbundlems.a \
    ${ohos_lib_path}/libcjson_static.a \
    ${ohos_lib_path}/libwant.a \
    ${ohos_lib_path}/libace_lite.a \
    ${ohos_lib_path}/libglobal_resmgr.a \
    ")
add_target_ohos_xts(ActsBundleMgrTest)
