#!/usr/bin/env bash

#
# Upload the .tar.gz and .xml artifacts to cloudsmith
#

STABLE_REPO=${CLOUDSMITH_STABLE_REPO\:-'jon-gough/testplugin_pi-stable'}
UNSTABLE_REPO=${CLOUDSMITH_UNSTABLE_REPO\:-'jon-gough/testplugin_pi'}

if [ "$(git rev-parse master)" != "$(git rev-parse HEAD)" ]; then
    echo "Not on master branch, skipping deployment."
    exit 0
fi

if [ -z "$CLOUDSMITH_API_KEY" ]; then
    echo 'Cannot deploy to cloudsmith, missing $CLOUDSMITH_API_KEY'
    exit 0
fi

echo "Using \$CLOUDSMITH_API_KEY: \$\{CLOUDSMITH_API_KEY\:0\:4\}..."

set -xe

python -m ensurepip
python -m pip install -q setuptools
python -m pip install -q cloudsmith-cli

commit=$(git rev-parse --short=7 HEAD) || commit="unknown"
now=$(date --rfc-3339=seconds) || now=$(date)


BUILD_ID=${APPVEYOR_BUILD_NUMBER:-1}
commit=$(git rev-parse --short=7 HEAD) || commit="unknown"
tag=$(git tag --contains HEAD)

tarball=$(ls *.tar.gz)
xml=$(ls *.xml)

source ../build/pkg_version.sh
test -n "$tag" && VERSION="$tag" || VERSION="${VERSION}+${BUILD_ID}.${commit}"
test -n "$tag" && REPO="$STABLE_REPO" || REPO="$UNSTABLE_REPO"

if [ -n "$tag" ]; then
    # There is no sed available in git bash. This is nasty, but seems
    # to work:
    while read line; do
        echo ${line/testplugin-pi/testplugin-stable}
    done < $xml > xml.tmp && cp xml.tmp $xml && rm xml.tmp
fi

cloudsmith push raw \
    --republish \
    --no-wait-for-sync \
    --name testplugin-linuxmint-19.2-metadata \
    --version  \
    --summary "testplugin opencpn plugin metadata for automatic installation" \
    $REPO $xml

cloudsmith push raw  \
    --republish \
    --no-wait-for-sync \
    --name testplugin-linuxmint-19.2-tarball \
    --version  \
    --summary "testplugin opencpn plugin tarball for automatic installation" \
    $REPO $tarball
