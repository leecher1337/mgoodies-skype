#define GEN_INIT_SUCCESS 0


typedef struct {
	int version;
	char *description;
	int (*init)();
	void (*config)();
	void (*quit)();
	HWND hwndParent;
	HINSTANCE hDllInstance;
} winampGeneralPurposePlugin;

#define GPPHDR_VER 0x10
#ifdef __cplusplus
extern "C" {
#endif
//extern winampGeneralPurposePlugin *gen_plugins[256];
typedef winampGeneralPurposePlugin * (*winampGeneralPurposePluginGetter)();
#ifdef __cplusplus
}
#endif