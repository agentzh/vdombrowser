#!/bin/sh

if [ -z $1 ]; then
    echo Version not specified on the command line.
    exit 1
fi
verison=$1

cd ..
vb_dir=VdomBrowser-$version
rm -rf $vb_dir
git clone VdomBrowser $vb_dir
tar -cvzf $vb_dir.tar.gz $vb_dir
cp $vb_dir.tar.gz `rpm -E '%{_sourcedir}'`

