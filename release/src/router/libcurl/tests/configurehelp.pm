# This is a generated file.  Do not edit.

package configurehelp;

use strict;
use warnings;
use Exporter;

use vars qw(
    @ISA
    @EXPORT_OK
    $Cpreprocessor
    );

@ISA = qw(Exporter);

@EXPORT_OK = qw(
    $Cpreprocessor
    );

$Cpreprocessor = 'mipsel-uclibc-gcc -E -Os -Wall -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32 -ffunction-sections -fdata-sections -isystem /raid1/tomato/tomato.git.115AC/release.ar.wp/src-rt/router/zlib';

1;
