import os
from SCons.Script import *

lsf_env = SConscript('build_core/SConscript')
SConscript('common/SConscript')

lsf_env['WS'] = None

if lsf_env['OS'] == 'openwrt':
    lsf_env.AppendUnique(LIBS = ['stdc++', 'pthread'])
elif lsf_env['BR'] == 'on':
    lsf_env.PrependUnique(LIBS = ['ajrouter', 'alljoyn'])

lsf_env.Append(CPPPATH = [lsf_env.Dir('$DISTDIR/cpp/inc')])

# Common files
header_depends = []
header_depends += lsf_env.Install('$DISTDIR/cpp/inc/alljoyn/lighting/', Glob('common/inc/alljoyn/lighting/*.h'))
header_depends += lsf_env.Install('$DISTDIR/cpp/inc/alljoyn/lighting/', Glob('standard_core_library/common/inc/alljoyn/lighting/*.h'))

# Common Lighting objects
lsf_env['common_srcs'] = lsf_env.Glob('standard_core_library/common/src/*.cc')
lsf_env['common_objs'] = lsf_env.Object(lsf_env['common_srcs'])
Depends(lsf_env['common_objs'], header_depends)

# Lighting Contoller Client
lsf_client_env = lsf_env.Clone()
header_depends += lsf_client_env.Install('$DISTDIR/cpp/inc/alljoyn/lighting/',
	Glob('standard_core_library/lighting_controller_client/inc/alljoyn/lighting/*.h'))
header_depends += lsf_client_env.Install('$DISTDIR/cpp/inc/alljoyn/lighting/client/',
	Glob('standard_core_library/lighting_controller_client/inc/alljoyn/lighting/client/*.h'))
lsf_client_env.Prepend(LIBS = ['alljoyn_notification', 'alljoyn_about', 'alljoyn_config', 'alljoyn_services_common'])
lsf_client_env['client_srcs'] = Glob('standard_core_library/lighting_controller_client/src/*.cc')
lsf_client_env['client_objs'] = lsf_client_env.Object(lsf_client_env['client_srcs'])
Depends(lsf_client_env['client_objs'], header_depends)
lsf_client_static_lib = lsf_client_env.StaticLibrary('$DISTDIR/cpp/lib/lighting_controller_client', 
    lsf_client_env['client_objs']
    + lsf_client_env['common_objs']
)
lsf_client_static_lib = lsf_client_env.SharedLibrary('$DISTDIR/cpp/lib/lighting_controller_client', 
    lsf_client_env['client_srcs']
    + lsf_client_env['common_srcs']
)
lsf_client_sample = lsf_client_env.Program('$DISTDIR/cpp/test/lighting_controller_client_sample',
    ['standard_core_library/lighting_controller_client/test/LightingControllerClientSample.cc'] 
    + lsf_client_env['client_objs']
    + lsf_client_env['common_objs']
)

# Lighting Controller Service
lsf_service_env = lsf_env.Clone()
header_depends += lsf_service_env.Install('$DISTDIR/cpp/inc/alljoyn/lighting/', 
        Glob('standard_core_library/lighting_controller_service/inc/alljoyn/lighting/*.h'))
header_depends += lsf_service_env.Install('$DISTDIR/cpp/inc/alljoyn/lighting/service/',
        Glob('standard_core_library/lighting_controller_service/inc/alljoyn/lighting/service/*.h'))
lsf_service_env.Prepend(LIBS = ['alljoyn_notification', 'alljoyn_about', 'alljoyn_config', 'alljoyn_services_common'])
lsf_service_env['service_srcs'] = Glob('standard_core_library/lighting_controller_service/src/*.cc')
lsf_service_env['service_objs'] = lsf_service_env.Object(lsf_service_env['service_srcs'])
Depends(lsf_service_env['service_objs'], header_depends)
lighting_controller_service = lsf_service_env.Program('$DISTDIR/cpp/bin/lighting_controller_service', 
    lsf_service_env['service_objs']
    + lsf_env['common_objs']
)
