#!/bin/sh

if [ -z "$1" ]; then
    echo Version not specified on the command line.
    exit 1
fi
version=$1

cd ..
vb_dir=VdomBrowser-$version
echo "$vb_dir"
echo "$version"
#exit
rm -rf $vb_dir
git clone VdomBrowser $vb_dir
tar -cvzf $vb_dir.tar.gz $vb_dir
echo cp $vb_dir.tar.gz `rpm -E '%{_sourcedir}'`
cp $vb_dir.tar.gz `rpm -E '%{_sourcedir}'`

