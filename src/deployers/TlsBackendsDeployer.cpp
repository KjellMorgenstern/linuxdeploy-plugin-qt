// library headers
#include <linuxdeploy/core/log.h>
#include <boost/filesystem.hpp>

// local headers
#include "TlsBackendsDeployer.h"

using namespace linuxdeploy::plugin::qt;
using namespace linuxdeploy::core::log;

namespace bf = boost::filesystem;

bool TlsBackendsDeployer::deploy() {
    // calling the default code is optional, but it won't hurt for now
    if (!BasicPluginsDeployer::deploy())
        return false;

    ldLog() << "Deploying TLS backends" << std::endl;

    for (bf::directory_iterator i(qtPluginsPath / "tls"); i != bf::directory_iterator(); ++i) {
        if (!appDir.deployLibrary(*i, appDir.path() / "usr/plugins/tls/"))
            return false;
    }

    return true;
}
