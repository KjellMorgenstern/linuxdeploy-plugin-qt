// library includes
#include <gtest/gtest.h>

// local includes
#include "../src/qml.h"
#include "../src/util.h"

namespace fs = std::filesystem;

namespace linuxdeploy {
    namespace plugin {
        namespace qt {
            namespace test {
                class TestDeployQml : public testing::Test {

                public:
                    std::filesystem::path appDirPath;
                    std::filesystem::path projectQmlRoot;
                    std::filesystem::path defaultQmlImportPath;

                    void SetUp() override {
                        appDirPath = getTempDirName();
                        projectQmlRoot = appDirPath.string() + "/usr/qml";

                        std::filesystem::create_directories(projectQmlRoot);
                        std::filesystem::copy_file(TESTS_DATA_DIR "/qml_project/file.qml",
                            projectQmlRoot.string() + "/file.qml");

                        setenv(ENV_KEY_QML_MODULES_PATHS, TESTS_DATA_DIR, 1);

                        defaultQmlImportPath = getQmlImportPath();
                    }

                    std::string getTempDirName() const {
                        char tmpl[] = "/tmp/linuxdeploy-plugin-qt-unit-tests-appdir-XXXXXX";
                        char* tempDir = mkdtemp(tmpl);

                        std::string name{tempDir};
                        return name;
                    }

                    void TearDown() override {
                        std::filesystem::remove_all(appDirPath);
                        unsetenv(ENV_KEY_QML_MODULES_PATHS);
                    }

                    fs::path getQmlImportPath() {
                        const auto& qmakePath = findQmake();
                        return queryQmake(qmakePath)["QT_INSTALL_QML"];
                    }
                };

                TEST_F(TestDeployQml, find_qmlimporter_path) {
                    auto result = findQmlImportScanner();
                    std::filesystem::path expected = "/usr/bin/qmlimportscanner";

                    ASSERT_FALSE(result.empty());
                    ASSERT_EQ(result.string(), expected.string());
                }

                TEST_F(TestDeployQml, runQmlImportScanner) {
                    auto result = runQmlImportScanner({projectQmlRoot},
                        {TESTS_DATA_DIR, defaultQmlImportPath});
                    ASSERT_FALSE(result.empty());
                    std::cout << result;
                }

                TEST_F(TestDeployQml, run_qmlimportscanner_without_qml_import_paths) {
                    auto result = runQmlImportScanner({projectQmlRoot}, {});
                    ASSERT_FALSE(result.empty());
                    std::cout << result;
                }

                // disabled, as Debian's qmlimportscanner does not behave as expected
                /*
                TEST_F(TestDeployQml, getQmlImports) {
                    auto results = getQmlImports(projectQmlRoot, defaultQmlImportPath);
                    ASSERT_FALSE(results.empty());
                    std::cout << "Imported Qml Modules Found:";
                    for (auto result : results) {
                        std::cout << "\n\tName: " << result.name << "\n\tPath: " << result.path << "\n\tRelative path:"
                                  << result.relativePath << std::endl;
                        ASSERT_FALSE(result.path.empty());
                        ASSERT_FALSE(result.relativePath.empty());
                    }
                }
                */

                TEST_F(TestDeployQml, deploy_qml_imports) {
                    linuxdeploy::core::appdir::AppDir appDir(appDirPath);

                    // speed up test runs; we don't check for the copyright files anyway
                    appDir.setDisableCopyrightFilesDeployment(true);

                    deployQml(appDir, defaultQmlImportPath);
                    appDir.executeDeferredOperations();

                    ASSERT_TRUE(std::filesystem::exists(projectQmlRoot.string() + "/QtQuick.2"));
                    ASSERT_TRUE(std::filesystem::exists(projectQmlRoot.string() + "/qml_module"));
                }

                TEST_F(TestDeployQml, getQmlModuleRelativePath) {
                    std::vector<fs::path> qmlModulesImportPaths = {"/usr/lib/x86_64-linux-gnu/qt5/qml", "/usr/lib/",
                                                                   "/usr/lib/qt5.10/qml", TESTS_DATA_DIR};

                    auto rpath = getQmlModuleRelativePath(qmlModulesImportPaths,
                        "/usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick.2");
                    ASSERT_EQ(rpath, "QtQuick.2");

                    rpath = getQmlModuleRelativePath(qmlModulesImportPaths,
                        "/usr/lib/x86_64-linux-gnu/qt5/qml/org/kde/plasma");
                    ASSERT_EQ(rpath, "org/kde/plasma");

                    rpath = getQmlModuleRelativePath(qmlModulesImportPaths, TESTS_DATA_DIR "/qml_module");
                    ASSERT_EQ(rpath, "qml_module");

                    rpath = getQmlModuleRelativePath(qmlModulesImportPaths, "/qml_module");
                    ASSERT_EQ(rpath, "");
                }
            }
        }
    }
}
