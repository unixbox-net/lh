#!/bin/bash

BASE_DIR=$(dirname "$(realpath "$0")")
RPM_BUILD_DIR="${BASE_DIR}/rpmbuild"

prepare_environment() {
    echo "Setting up build environment..."
    mkdir -p "${RPM_BUILD_DIR}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}"
    cp "${BASE_DIR}/src/"*.c "${RPM_BUILD_DIR}/SOURCES/"
    tar czf "${RPM_BUILD_DIR}/SOURCES/lh-1.0.0.tar.gz" -C "${BASE_DIR}/src/" .
    cp "${BASE_DIR}/lh.spec" "${RPM_BUILD_DIR}/SPECS/"
}

build_rpm() {
    echo "Building RPM package..."
    rpmbuild -ba "${RPM_BUILD_DIR}/SPECS/lh.spec" --define "_topdir ${RPM_BUILD_DIR}"
}

prepare_environment
build_rpm
