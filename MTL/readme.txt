MTL - Miranda Template Library for Miranda IM


Copyright 2007 Paul Shmakov


INTRODUCTION

        MTL is a tiny C++ library that offers C++ development way for
        Miranda IM.
        MTL is based on the same concepts as ATL and WTL libraries.

        Miranda uses C-style API. A notion of Service and Event is used for
        all the communication between the core and the modules/plugins.
        Each Service/Event is a named C static function with two untyped
        (WPARAM & LPARAM) arguments.
        This leads to a "stateless" nature of the services and events - they
        can't save their state between calls by any means, but global variables.

        The main idea behind MTL - a lightweight object-oriented wrapper
        around Miranda API. It offers an ability to create the "stateful"
        services and event handlers by employing objects and their methods.


        Here's a basic sample:

        // Begin of test.cpp

        #include <mtl.h>

        // Test class: counts the changes in the database
        class CDatabaseWatcher
        {
        public:
            CDatabaseWatcher() : m_changes(0)
            {
            }
            void Init()
            {
                HookEvent(ME_DB_CONTACT_SETTINGCHANGED, this,
                    &CDatabaseWatcher::OnDbSettingChanged);
            }
        private:
            int OnDbSettingChanged(HANDLE hContact, DBCONTACTWRITESETTING* cws)
            {
                ++m_changes;
                return 0;
            }

            int m_changes;
        };

        // Plugin class
        class CMyPlugin : public CMirandaPlugin<CMyPlugin>
        {
        private:
            CDatabaseWatcher m_dbWatcher;

            BEGIN_PLUGIN_INFO(CMyPlugin)
                PLUGIN_NAME        ("Singer")
                PLUGIN_VERSION     (PLUGIN_MAKE_VERSION(1,0,0,0))
                PLUGIN_DESCRIPTION ("Sings incoming messages")
                // Other properties
            END_PLUGIN_INFO()

            // Sample event handler - non-static (!) method
            int OnModulesLoaded(WPARAM, LPARAM)
            {
                m_dbWatcher.Init();
                // ...
                return 0;
            }

            // Sample service - non-static (!) method
            // Note the types of parameters. Certainly, we can use
            // WPARAM & LPARAM types here, but we don't have to!
            // This helps to get rid of ugly casts and to make an interface
            // strongly typed.
            int SingService(const char* message, int voiceType)
            {
                // ...
                return 0;
            }

            // Another sample service - non-static method as well
            //
            // Let's say, this service is NOT thread safe, and
            // must be called in the main thread context only -
            // for example, it uses some unsafe COM objects.
            //
            // MTL offers an ability to ensure that this service
            // will be called in the main thread context only -
            // look at OnLoad() for details
            int MakeCofeeService(int numberOfCups, LPARAM)
            {
                // ...
                return 0;
            }

            int OnLoad()
            {
                CreateService("MyPlugin/SingMessage", &CMyPlugin::SingService);
                HookEvent(ME_SYSTEM_MODULESLOADED, &CMyPlugin::OnModulesLoaded);
                CreateMainThreadService("MyPlugin/CoffeePlease", &CMyPlugin::MakeCofeeService);

                // Note the use of CreateMainThreadService instead of infamous
                // CreateService for the "MyPlugin/CoffeePlease" service.
                // CreateMainThreadService garanties that the specified service
                // will be called in the main thread context only.

                // ...
                return 0;
            }
        };

        //
        // Obligatory part
        //

        CMirandaPluginLink g_pluginLink;
        CMyPlugin g_plugin;

        BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
        {
            return g_plugin.DllMain(hInst, reason);
        }

        extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
        {
            return g_plugin.MirandaPluginInfo(mirandaVersion);
        }

        extern "C" int __declspec(dllexport) Load(PLUGINLINK* link)
        {
            return g_plugin.Load(link);
        }

        extern "C" int __declspec(dllexport) Unload(void)
        {
            return g_plugin.Unload();
        }

        // End of test.cpp


        That's it. Several things to note:

        1. The plugin must be derived from CMirandaPlugin base class.

        2. The SingService and OnModulesLoaded methods are non-static member
           functions, so they can access member variables and methods.

        3. Services and Event handlers don't have to match
           int (*)(WPARAM,LPARAM) prototype. The types of return value and
           parameters can be any compatible (i.e. same size) types. For example,
           HANDLE AddToListFromSearch(int flags, PROTOSEARCHRESULT*).

        4. There can be a lot of objects that hook events and/or expose their
           services.


KNOWN ISSUES / LIMITATIONS

        - No x64, ia64 support. Miranda IM doesn't support them as well.

        - Multiple class inheritance is not supported.
