// gmi_auxiliary_macros.h
// define auxiliary macro for some module
#if !defined( GMI_AUXILIARY_MACRO )
#define GMI_AUXILIARY_MACRO

//avoid compiling warning on ambarella_a5s_sdk_v3.3
#define READ_MYSELF(x) ((x)=(x))

#endif//GMI_AUXILIARY_MACRO
