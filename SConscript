import os

Import('env')

env['_ALLJOYN_LSF_'] = True

#Build Lamp Service
lamp_service_env = env.Clone()
lamp_service_env.Append(LIBPATH = [ lamp_service_env.Dir('../ajtcl') ])
lamp_service_env.Prepend(LIBS = ['libajtcl.a'])
lamp_service_env.Append(CPPPATH = [ lamp_service_env.Dir('thin_core_library/lamp_service/inc'),
                                    lamp_service_env.Dir('common/inc'),
                                    lamp_service_env.Dir('../../base_tcl/sample_apps/AppsCommon/inc'),
                                    lamp_service_env.Dir('../../base_tcl/services_common/inc'),
                                    lamp_service_env.Dir('../../base_tcl/config/inc'),
                                    lamp_service_env.Dir('../../base_tcl/notification/inc')
                                    ])

lamp_service_env['LSF_LAMP_DISTDIR'] = 'build/linux/thin_core_library/lamp_service'
lamp_service_env.Install('$LSF_LAMP_DISTDIR/inc', lamp_service_env.Glob('thin_core_library/lamp_service/inc/*.h'))

#lamp_service_env.Append(CPPDEFINES = ['CONFIG_SERVICE', 'NOTIFICATION_SERVICE_PRODUCER'])

srcs = [f for f in lamp_service_env.Glob('thin_core_library/lamp_service/src/*.c') if not (str(f).endswith('LampMain.c'))]
lamp_service_env['lamp_srcs'] = srcs

# the LampService sources
objs = lamp_service_env.Object(lamp_service_env['lamp_srcs'])

# services 
objs += SConscript('../../base_tcl/services_common/SConscript', {'services_common_env': lamp_service_env})
objs += SConscript('../../base_tcl/config/SConscript', {'config_env': lamp_service_env})
objs += SConscript('../../base_tcl/notification/src/NotificationCommon/SConscript', {'notif_env': lamp_service_env})
objs += SConscript('../../base_tcl/notification/src/NotificationProducer/SConscript', {'notif_env': lamp_service_env})

# for the PropertyStore:
objs += SConscript('../../base_tcl/sample_apps/AppsCommon/SConscript', {'appsCommon_env': lamp_service_env})

lamp_service_env['lamp_objs'] = objs;

lamp_service = lamp_service_env.Program('$LSF_LAMP_DISTDIR/bin/lamp_service', ['thin_core_library/lamp_service/src/LampMain.c'] + lamp_service_env['lamp_objs'])
lamp_service_env.Install('$LSF_LAMP_DISTDIR/bin', lamp_service_env['lamp_objs'])
lamp_service_env.Install('$LSF_LAMP_DISTDIR/bin', 'thin_core_library/lamp_service/test/LaunchLampServices.sh')






