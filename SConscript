import os

Import('env')

env['_ALLJOYN_LSF_'] = True

if not env.has_key('_ALLJOYNCORE_') and os.path.exists('../alljoyn/alljoyn_core/SConscript'):
   env.SConscript('../alljoyn/alljoyn_core/SConscript')

# Do not build samples for services  
env['BUILD_SERVICES_SAMPLES'] = 'off'

# Add all services include path
env.Append(CPPPATH = '$DISTDIR/about/inc');
env.Append(CPPPATH = '$DISTDIR/config/inc');
env.Append(CPPPATH = '$DISTDIR/services_common/inc');
env.Append(CPPPATH = '$DISTDIR/notification/inc');

# Build all the services
env.SConscript('../alljoyn/services/about/cpp/SConscript', {'about_env': env})
env.SConscript('../../base/config/cpp/SConscript', {'config_env': env})
env.SConscript('../../base/notification/cpp/SConscript', {'nsenv': env})
env.SConscript('../../base/services_common/cpp/SConscript', {'services_common_env': env})

lsf_env = env.Clone()
lsf_env['WS'] = None 
 
# Add all services lib path
lsf_env.Append(LIBPATH = '$DISTDIR/about/lib')    
lsf_env.Append(LIBPATH = '$DISTDIR/services_common/lib')  
lsf_env.Append(LIBPATH = '$DISTDIR/notification/lib') 
lsf_env.Append(LIBPATH = '$DISTDIR/config/lib') 

if lsf_env['BR'] == 'on' :
    bdFile = lsf_env.subst('$DISTDIR') + '/cpp/lib/BundledRouter.o'
    lsf_env.Prepend(LIBS = [File(bdFile), 'ajrouter'])
if lsf_env['OS'] == 'openwrt':
    lsf_env.AppendUnique(LIBS = [ 'stdc++', 'pthread' ])

lsf_env.Install('$DISTDIR/cpp/bin', 'thin_core_library/lamp_service/routerconfig.xml')
    
# Build all the LSF Common objects   
lsf_env.Append(CPPPATH = [ lsf_env.Dir('standard_core_library/common/inc') ])
lsf_env.Append(CPPPATH = [ lsf_env.Dir('common/inc') ])
lsf_env['common_srcs'] = lsf_env.Glob('standard_core_library/common/src/*.cc')
#lsf_env['common_srcs'] += lsf_env.Glob('../../base/sample_apps/cpp/samples_common/*.cc')
lsf_env['common_objs'] = lsf_env.Object(lsf_env['common_srcs']) 

#Build Lighting Controller Client
lsf_client_env = lsf_env.Clone()
lsf_client_env.Append(CPPPATH = [ lsf_client_env.Dir('standard_core_library/lighting_controller_client/inc') ])
lsf_client_env.Prepend(LIBS = ['alljoyn_notification.a', 'alljoyn_about.a', 'alljoyn_config.a', 'alljoyn_services_common.a'])
lsf_client_env['LSF_CLIENT_DISTDIR'] = 'build/linux/standard_core_library/lighting_controller_client'
lsf_client_env.Install('$LSF_CLIENT_DISTDIR/inc', lsf_client_env.Glob('standard_core_library/common/inc/*.h'))
lsf_client_env.Install('$LSF_CLIENT_DISTDIR/inc', lsf_client_env.Glob('standard_core_library/lighting_controller_client/inc/*.h'))
lsf_client_env['client_srcs'] = lsf_client_env.Glob('standard_core_library/lighting_controller_client/src/*.cc')
lsf_client_env['client_objs'] = lsf_client_env.Object(lsf_client_env['client_srcs']) 
lighting_controller_client_static_lib = lsf_client_env.StaticLibrary('$LSF_CLIENT_DISTDIR/lib/lighting_controller_client', lsf_client_env['client_objs'] + lsf_env['common_objs']);
lighting_controller_client_sample = lsf_client_env.Program('$LSF_CLIENT_DISTDIR/test/lighting_controller_client_sample', ['standard_core_library/lighting_controller_client/test/LightingControllerClientSample.cc'] + lsf_client_env['client_objs'] + lsf_env['common_objs'])
lsf_client_env.Install('$LSF_CLIENT_DISTDIR/bin', lsf_client_env['client_objs'])
lsf_client_env.Install('$LSF_CLIENT_DISTDIR/bin', lsf_env['common_objs'])

# Build a test for the Lamp Service - Lamp Service Test needs to be updated to use the latest defs in LSFTypes - Turn off building until then
#lamp_test_env = lsf_client_env.Clone()
#lamp_test_env.Append(LIBPATH = '$LSF_CLIENT_DISTDIR/lib')
#lamp_test_env.Prepend(LIBS = ['lighting_controller_client.a'])
#lamp_test_env.Append(LINKFLAGS = ['-pthread'])
#lamp_test_env['LSF_LAMP_TEST_DISTDIR'] = 'build/linux/standard_core_library/lighting_controller_client/test/'
#lamp_test_env.Program('$LSF_LAMP_TEST_DISTDIR/lamp_service_test', ['standard_core_library/lighting_controller_client/test/LampServiceTest.cc'])

#Build Lighting Controller Service
lsf_service_env = lsf_env.Clone()
lsf_service_env.Append(CPPPATH = [ lsf_service_env.Dir('standard_core_library/lighting_controller_service/inc') ])
lsf_service_env.Prepend(LIBS = ['alljoyn_notification.a', 'alljoyn_about.a', 'alljoyn_config.a', 'alljoyn_services_common.a'])
lsf_service_env['LSF_SERVICE_DISTDIR'] = 'build/linux/standard_core_library/lighting_controller_service'
lsf_service_env.Install('$LSF_SERVICE_DISTDIR/inc', lsf_service_env.Glob('standard_core_library/common/inc/*.h'))
lsf_service_env.Install('$LSF_SERVICE_DISTDIR/inc', lsf_service_env.Glob('standard_core_library/lighting_controller_service/inc/*.h'))
lsf_service_env['service_srcs'] = [f for f in lsf_service_env.Glob('standard_core_library/lighting_controller_service/src/*.cc') if not (str(f).endswith('Main.cc'))]
lsf_service_env['service_objs'] = lsf_service_env.Object(lsf_service_env['service_srcs'])
lighting_controller_service = lsf_service_env.Program('$LSF_SERVICE_DISTDIR/bin/lighting_controller_service', ['standard_core_library/lighting_controller_service/src/Main.cc'] + lsf_service_env['service_objs'] + lsf_env['common_objs'])
lsf_service_env.Install('$LSF_SERVICE_DISTDIR/bin', lsf_service_env['service_objs'])
lsf_service_env.Install('$LSF_SERVICE_DISTDIR/bin', lsf_env['common_objs'])
lsf_service_env.Install('$LSF_SERVICE_DISTDIR/bin', 'standard_core_library/lighting_controller_service/src/OEMConfig.ini')

#Build Lamp Service
lamp_service_env = SConscript('../ajtcl/SConscript')
lamp_service_env.Append(LIBPATH = [ lamp_service_env.Dir('../ajtcl') ])
lamp_service_env.Append(CPPPATH = [ lamp_service_env.Dir('thin_core_library/lamp_service/inc'),
                                    lamp_service_env.Dir('common/inc'),
                                    lamp_service_env.Dir('../../base_tcl/services_common/inc'),
                                    lamp_service_env.Dir('../../base_tcl/config/inc'),
                                    lamp_service_env.Dir('../../base_tcl/notification/inc')
                                    ])

lamp_service_env['LSF_LAMP_DISTDIR'] = 'build/linux/thin_core_library/lamp_service'
lamp_service_env.Install('$LSF_LAMP_DISTDIR/inc', lamp_service_env.Glob('thin_core_library/lamp_service/inc/*.h'))

srcs = [f for f in lamp_service_env.Glob('thin_core_library/lamp_service/src/*.c') if not (str(f).endswith('LampMain.c'))]
lamp_service_env['lamp_srcs'] = srcs

# the LampService sources
objs = lamp_service_env.Object(lamp_service_env['lamp_srcs'])

# the AJTCL sources
objs += lamp_service_env['aj_obj']

# services 
objs += SConscript('../../base_tcl/services_common/SConscript', {'services_common_env': lamp_service_env})
objs += SConscript('../../base_tcl/config/SConscript', {'config_env': lamp_service_env})
objs += SConscript('../../base_tcl/notification/src/NotificationCommon/SConscript', {'notif_env': lamp_service_env})
objs += SConscript('../../base_tcl/notification/src/NotificationProducer/SConscript', {'notif_env': lamp_service_env})

lamp_service_env['lamp_objs'] = objs;

lamp_service = lamp_service_env.Program('$LSF_LAMP_DISTDIR/bin/lamp_service', ['thin_core_library/lamp_service/src/LampMain.c'] + lamp_service_env['lamp_objs'])
lamp_service_env.Install('$LSF_LAMP_DISTDIR/bin', lamp_service_env['lamp_objs'])
lamp_service_env.Install('$LSF_LAMP_DISTDIR/bin', 'thin_core_library/lamp_service/test/LaunchLampServices.sh')


