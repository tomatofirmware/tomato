#!/usr/bin/perl
#
#	libfoo.pl
#	Copyright (C) 2006-2008 Jonathan Zarate
#
#	- strip un-needed objects
#	- create xref of symbols used
#

$root = $ENV{"TARGETDIR"};
$uclibc = $ENV{"TOOLCHAIN"};
$router = $ENV{"SRCBASE"} . "/router";

sub error
{
	print STDERR "\n*** ERROR: " . (shift) . "\n\n";
	exit 1;
}

sub basename
{
	my $fn = shift;
	if ($fn =~ /([^\/]+)$/) {
		return $1;
	}
	return $fn;
}
sub dirname
{
        my $dn;
        my $lastc;
        my $fn = shift;
        $dn = substr($fn, 0, length($fn)-length(basename($fn)));
        $lastc = substr($dn, length($dn)-1, length($dn)-1);
        if ($lastc = "/") {
                $dn = substr($dn, 0, length($dn)-1);
        }
        return $dn;
}
sub load
{
    my $fname = shift;

	if ((-l $fname) ||
		($fname =~ /\/lib\/modules\/\d+\.\d+\.\d+/) ||
		($fname =~ /\.(asp|gif|png|svg|js|jsx|css|txt|pat|sh)$/)) {
		return;
	}
	
	if (-d $fname) {
		my $d;
		if (opendir($d, $fname)) {
			foreach (readdir($d)) {
				if ($_ !~ /^\./) {
					load($fname . "/" . $_);
				}
			}
			closedir($d);
		}
		return;
	}


	my $f;
	my $base;
	my $ok;
	my $s;

	$base = basename($fname);
	print LOG "\n\nreadelf $base:\n";
	
	open($f, "mipsel-linux-readelf -WhsdD ${fname} 2>&1 |") || error("readelf - $!\n");
	
	while (<$f>) {
		print LOG;

		if (/\s+Type:\s+(\w+)/) {
			$elf_type{$base} = $1;
			$ok = 1;
			last;
		}
	}
	
	if (!$ok) {
		close($f);
		return;
	}

	print "$elf_type{$base} $base", " " x 30, "\r";
	
	push(@elfs, $base);
	
	while (<$f>) {
		print LOG;
		
		if (/\(NEEDED\)\s+Shared library: \[(.+)\]/) {
			push(@{$elf_lib{$base}}, $1);
		}
		elsif (/Symbol table for image:/) {
			last;
		}
	}
	
	while (<$f>) {
		print LOG;

		if (/\s+(WEAK|GLOBAL)\s+(?:DEFAULT|VISIBLE)\s+(\w+)\s+(\w+)/) {
			$s = $3;
			if ($2 eq 'UND') {
				if ($1 eq 'GLOBAL') {
					$elf_ext{$base}{$s} = 1;
				}
				else {
					print LOG "*** not GLOBAL\n";
				}
			}
			elsif ($2 eq 'ABS') {
			}
			elsif ($2 =~ /^\d+$/) {
				$elf_exp{$base}{$s} = 1;
			}
			else {
				print LOG "*** unknown type\n";
			}
		}
		elsif (!/Num Buc:/) {
			print LOG "*** strange line\n";
		}
	}

	close($f);
}

sub fixDynDep
{
	my ($user, $dep) = @_;
	
	if (!defined $elf_dyn{$user}{$dep}) {
		push(@{$elf_lib{$user}}, $dep);
		$elf_dyn{$user}{$dep} = 1;

		print LOG "FixDynDep: $user = $dep\n";
	}
}

sub fixDyn
{
	my $s;

	foreach (@elfs) {
		if (/^libipt_.+\.so$/) {
			fixDynDep("iptables", $_);
		}
		elsif (/^libip6t_.+\.so$/) {
			fixDynDep("ip6tables", $_);
		}
		elsif (/^CP\d+\.so$/) {
			fixDynDep("smbd", $_);
		}
	}

	fixDynDep("l2tpd", "cmd.so");
	fixDynDep("l2tpd", "sync-pppd.so");
	fixDynDep("pppd", "pppol2tp.so");
	fixDynDep("pppd", "pptp.so");
	fixDynDep("pppd", "rp-pppoe.so");

	fixDynDep("libcrypto.so.1.0.0", "libssl.so.1.0.0");

#shibby
	fixDynDep("transmission-daemon", "libevent-2.0.so.5");
	fixDynDep("transmission-daemon", "libcurl.so.4.3.0");
#	fixDynDep("transmission-daemon", "libiconv.so.2");
	fixDynDep("transmission-remote", "libevent-2.0.so.5");
	fixDynDep("transmission-remote", "libcurl.so.4.3.0");
#	fixDynDep("transmission-remote", "libiconv.so.2");
	fixDynDep("radvd", "libdaemon.so.0.5.0");
	fixDynDep("miniupnpd", "libnfnetlink.so.0.2.0");
	fixDynDep("dnscrypt-proxy", "libsodium.so.4.5.0");

#aria2 module, bwq518
	fixDynDep("aria2c", "libcares.so.2.0.0");
	fixDynDep("aria2c", "libexpat.so.1.6.0");
	fixDynDep("aria2c", "libiconv.so.2.4.0");
	fixDynDep("aria2c", "libstdc.so.6");
	fixDynDep("aria2c", "libssl.so.1.0.0");
	fixDynDep("aria2c", "libz.so.1");
	fixDynDep("aria2c", "libsqlite3.so.0");
#wget module, bwq518
	fixDynDep("wgetpro", "libz.so.1");
	fixDynDep("wgetpro", "libstdc.so.6");
	fixDynDep("wgetpro", "libiconv.so.2.4.0");
	fixDynDep("wgetpro", "libssl.so.1.0.0");
#minidlna module, bwq518
	fixDynDep("minidlna", "libz.so.1");
	fixDynDep("minidlna", "libstdc.so.6");
	fixDynDep("minidlna", "libiconv.so.2.4.0");
	fixDynDep("minidlna", "libssl.so.1.0.0");
	fixDynDep("minidlna", "libjpeg.so");
	fixDynDep("minidlna", "libogg.so.0");
	fixDynDep("minidlna", "libvorbis.so.0");
	fixDynDep("minidlna", "libid3tag.so.0");
	fixDynDep("minidlna", "libexif.so.12");
	fixDynDep("minidlna", "libFLAC.so.8");
	fixDynDep("libjepg.so", "libc.so.0");
	fixDynDep("libavcodec.so.52", "libpthread.so.0");
#python module, bwq518
	fixDynDep("python2.7", "libpython2.7.so.1.0");
	fixDynDep("python2.7", "libreadline.so.6.2");
	fixDynDep("python2.7", "libsqlite3.so.0");
	fixDynDep("python2.7", "libncurses.so.5");
	fixDynDep("python2.7", "libcurl.so.4.3.0");
	fixDynDep("python2.7", "libbz2.so.1.0");
	fixDynDep("python2.7", "libev.so.4.0.0");
	fixDynDep("python2.7", "libcares.so.2.0.0");
	fixDynDep("python2.7", "libexpat.so.1.6.0");
	fixDynDep("python2.7", "libstdc.so.6");
	fixDynDep("python2.7", "libssl.so.1.0.0");
	fixDynDep("python2.7", "libz.so.1");
	fixDynDep("libpython2.7.so.1.0", "libbz2.so.1.0");
	fixDynDep("libpython2.7.so.1.0", "libdb-4.8.so");
	fixDynDep("libpython2.7.so.1.0", "libpanel.so.5");
	fixDynDep("_curses_panel.so", "libpanel.so.5");
	fixDynDep("readline.so", "libreadline.so.6.2");
	fixDynDep("_ctypes.so", "libdl.so.0");
	fixDynDep("crypt.so", "libcrypt.so.0");
	fixDynDep("_multiprocessing.so", "libpthread.so.0");
	fixDynDep("_json.so", "libm.so.0");
	fixDynDep("_ctypes_test.so", "libm.so.0");
	fixDynDep("audioop.so", "libm.so.0");
	fixDynDep("pyexpat.so", "libexpat.so.1.6.0");
	fixDynDep("core.so", "libev.so.4.0.0");
	fixDynDep("core.so", "libcares.so.2.0.0");
	fixDynDep("ares.so", "libcares.so.2.0.0");

	fixDynDep("arraymodule.so", "libpython2.7.so.1.0");
	fixDynDep("audioop.so", "libpython2.7.so.1.0");
	fixDynDep("binascii.so", "libpython2.7.so.1.0");
	fixDynDep("_bisectmodule.so", "libpython2.7.so.1.0");
	fixDynDep("bsddb185module.so", "libpython2.7.so.1.0");
	fixDynDep("bsddb185module.so", "libdb-4.8.so");
	fixDynDep("_bsddb.so", "libpython2.7.so.1.0");
	fixDynDep("_bsddb.so", "libdb-4.8.so");
	fixDynDep("bz2module.so", "libpython2.7.so.1.0");
	fixDynDep("bz2module.so", "libbz2.so.1.0");
	fixDynDep("cmathmodule.so", "libpython2.7.so.1.0");
	fixDynDep("cmathmodule.so", "libm.so.0");
	fixDynDep("_codecs_cn.so", "libpython2.7.so.1.0");
	fixDynDep("_codecs_hk.so", "libpython2.7.so.1.0");
	fixDynDep("_codecs_iso2022.so", "libpython2.7.so.1.0");
	fixDynDep("_codecs_jp.so", "libpython2.7.so.1.0");
	fixDynDep("_codecs_kr.so", "libpython2.7.so.1.0");
	fixDynDep("_codecs_tw.so", "libpython2.7.so.1.0");
	fixDynDep("_collectionsmodule.so", "libpython2.7.so.1.0");
	fixDynDep("cPickle.so", "libpython2.7.so.1.0");
	fixDynDep("cryptmodule.so", "libpython2.7.so.1.0");
	fixDynDep("cryptmodule.so", "libcrypt.so.0");
	fixDynDep("cStringIO.so", "libpython2.7.so.1.0");
	fixDynDep("_csv.so", "libpython2.7.so.1.0");
	fixDynDep("_ctypes.so", "libpython2.7.so.1.0");
	fixDynDep("_ctypes_test.so", "libpython2.7.so.1.0");
	fixDynDep("_cursesmodule.so", "libncurses.so.5");
	fixDynDep("_cursesmodule.so", "libpython2.7.so.1.0");
	fixDynDep("_curses_panel.so", "libpython2.7.so.1.0");
	fixDynDep("datetimemodule.so", "libpython2.7.so.1.0");
	fixDynDep("datetimemodule.so", "libc.so.0");
	fixDynDep("datetimemodule.so", "libm.so.0");
	fixDynDep("datetimemodule.so", "timemodule.so");
	fixDynDep("dlmodule.so", "libpython2.7.so.1.0");
	fixDynDep("dlmodule.so", "libdl.so.0");
	fixDynDep("_elementtree.so", "libpython2.7.so.1.0");
	fixDynDep("fcntlmodule.so", "libpython2.7.so.1.0");
	fixDynDep("_functoolsmodule.so", "libpython2.7.so.1.0");
	fixDynDep("future_builtins.so", "libpython2.7.so.1.0");
	fixDynDep("grpmodule.so", "libpython2.7.so.1.0");
	fixDynDep("_heapq.so", "libpython2.7.so.1.0");
	fixDynDep("_hotshot.so", "libpython2.7.so.1.0");
	fixDynDep("imageop.so", "libpython2.7.so.1.0");
	fixDynDep("_io.so", "libpython2.7.so.1.0");
	fixDynDep("itertoolsmodule.so", "libpython2.7.so.1.0");
	fixDynDep("_json.so", "libpython2.7.so.1.0");
	fixDynDep("linuxaudiodev.so", "libpython2.7.so.1.0");
	fixDynDep("_localemodule.so", "libpython2.7.so.1.0");
	fixDynDep("_lsprof.so", "libpython2.7.so.1.0");
	fixDynDep("mathmodule.so", "libpython2.7.so.1.0");
	fixDynDep("mathmodule.so", "libm.so.0");
	fixDynDep("_md5module.so", "libpython2.7.so.1.0");
	fixDynDep("mmapmodule.so", "libpython2.7.so.1.0");
	fixDynDep("_multibytecodecmodule.so", "libpython2.7.so.1.0");
	fixDynDep("_multiprocessing.so", "libpython2.7.so.1.0");
	fixDynDep("operator.so", "libpython2.7.so.1.0");
	fixDynDep("ossaudiodev.so", "libpython2.7.so.1.0");
	fixDynDep("parsermodule.so", "libpython2.7.so.1.0");
	fixDynDep("pyexpat.so", "libpython2.7.so.1.0");
	fixDynDep("pyexpat.so", "libexpat.so.1.6.0");
	fixDynDep("_randommodule.so", "libpython2.7.so.1.0");
	fixDynDep("readline.so", "libpython2.7.so.1.0");
	fixDynDep("resource.so", "libpython2.7.so.1.0");
	fixDynDep("selectmodule.so", "libpython2.7.so.1.0");
	fixDynDep("_sha256module.so", "libpython2.7.so.1.0");
	fixDynDep("_sha512module.so", "libpython2.7.so.1.0");
	fixDynDep("_shamodule.so", "libpython2.7.so.1.0");
	fixDynDep("_socketmodule.so", "libpython2.7.so.1.0");
	fixDynDep("spwdmodule.so", "libpython2.7.so.1.0");
	fixDynDep("_sqlite3.so", "libpython2.7.so.1.0");
	fixDynDep("_sqlite3.so", "libsqlite3.so.0");
	fixDynDep("_ssl.so", "libpython2.7.so.1.0");
	fixDynDep("_ssl.so", "libssl.so.1.0.0");
	fixDynDep("_ssl.so", "libcrypto.so.1.0.0");
	fixDynDep("stropmodule.so", "libpython2.7.so.1.0");
	fixDynDep("_struct.so", "libpython2.7.so.1.0");
	fixDynDep("syslogmodule.so", "libpython2.7.so.1.0");
	fixDynDep("termios.so", "libpython2.7.so.1.0");
	fixDynDep("_testcapi.so", "libpython2.7.so.1.0");
	fixDynDep("timemodule.so", "libpython2.7.so.1.0");
	fixDynDep("timemodule.so", "libc.so.0");
	fixDynDep("timemodule.so", "libm.so.0");
	fixDynDep("unicodedata.so", "libpython2.7.so.1.0");
	fixDynDep("xxsubtype.so", "libpython2.7.so.1.0");
	fixDynDep("zlibmodule.so", "libpython2.7.so.1.0");
	fixDynDep("zlibmodule.so", "libz.so.1");
#iptables, bwq518
	fixDynDep("xtables-multi", "libxtables.so.10.0.0");
	fixDynDep("xtables-multi", "libiptc.so.0.0.0");
	fixDynDep("xtables-multi", "libip4tc.so.0.1.0");
	fixDynDep("xtables-multi", "libip6tc.so.0.1.0");
	fixDynDep("nfnl_osf", "libnfnetlink.so.0.2.0");
	foreach (@elfs) {
		if (/^libipt_.\.so$/) {
			fixDynDep("xtables-multi", $_);
			fixDynDep("libxtables.so.10.0.0", $_);
			fixDynDep("libiptc.so.0.0.0", $_);
			fixDynDep("libip4tc.so.0.1.0", $_);
			fixDynDep("libip6tc.so.0.1.0", $_);
		}
		elsif (/^libip6t_.\.so$/) {
			fixDynDep("xtables-multi", $_);
			fixDynDep("libxtables.so.10.0.0", $_);
			fixDynDep("libiptc.so.0.0.0", $_);
			fixDynDep("libip4tc.so.0.1.0", $_);
			fixDynDep("libip6tc.so.0.1.0", $_);
		}
		elsif (/^libxt_.\.so$/) {
			fixDynDep("xtables-multi", $_);
			fixDynDep("libxtables.so.10.0.0", $_);
			fixDynDep("libiptc.so.0.0.0", $_);
			fixDynDep("libip4tc.so.0.1.0", $_);
			fixDynDep("libip6tc.so.0.1.0", $_);
		}
	}
	fixDynDep("miniupnpd", "libip4tc.so.0.1.0");
 
#ipset modules
	fixDynDep("libipset_iphash.so", "ipset");
	fixDynDep("libipset_iptree.so", "ipset");
	fixDynDep("libipset_ipmap.so", "ipset");
	fixDynDep("libipset_ipporthash.so", "ipset");
	fixDynDep("libipset_ipportiphash.so", "ipset");
	fixDynDep("libipset_ipportnethash.so", "ipset");
	fixDynDep("libipset_iptreemap.so", "ipset");
	fixDynDep("libipset_macipmap.so", "ipset");
	fixDynDep("libipset_nethash.so", "ipset");
	fixDynDep("libipset_portmap.so", "ipset");
	fixDynDep("libipset_setlist.so", "ipset");

	fixDynDep("tomatodata.cgi", "libc.so.0");
	fixDynDep("tomatoups.cgi", "libc.so.0");
	fixDynDep("apcupsd", "libc.so.0");
	fixDynDep("apcupsd", "libgcc_s.so.1");
	fixDynDep("apcaccess", "libc.so.0");
	fixDynDep("smtp", "libc.so.0");

#	fixDynDep("libbcm.so", "libshared.so");
#	fixDynDep("libbcm.so", "libc.so.0");
#for get_ifname_by_wlmac,wl_wlif_is_psta
	fixDynDep("eapd", "libshared.so");
#for nvram_default_get
	fixDynDep("wlconf", "libshared.so");
	fixDynDep("libusb-1.0.so", "libc.so.0");
	fixDynDep("libusb-1.0.so", "libutil.so.0");
	fixDynDep("pptpctrl", "libutil.so.0");

#!!TB - Updated Broadcom WL driver
	fixDynDep("libbcmcrypto.so", "libc.so.0");
	fixDynDep("nas", "libbcmcrypto.so");
	fixDynDep("wl", "libbcmcrypto.so");
	fixDynDep("nas", "libc.so.0");
	fixDynDep("wl", "libc.so.0");

#Roadkill for NocatSplash
	fixDynDep("splashd","libglib-1.2.so.0.0.10");
}

sub usersOf
{
	my $name = shift;
	my $sym = shift;
	my @x;
	my $e;
	my $l;
	
	@x = ();
	foreach $e (@elfs) {
		foreach $l (@{$elf_lib{$e}}) {
			if ($l eq $name) {
				if ((!defined $sym) || (defined $elf_ext{$e}{$sym})) {
					push(@x, $e);
				}
				last;
			}
		}
	}
	return @x;
}

sub resolve
{
	my $name = shift;
	my $sym = shift;
	my $l;
	
	foreach $l (@{$elf_lib{$name}}) {
#		print "\n$l $sym ", $elf_exp{$l}{$sym}, "\n";
		return $l if (defined $elf_exp{$l}{$sym});
	}
	return "*** unresolved ***";
}

sub fillGaps
{
	my $name;
	my $sym;
	my @users;
	my $u;
	my $t;
	my $found;

	print "Resolving implicit links...\n";
	
	foreach $name (@elfs) {
		foreach $sym (keys %{$elf_ext{$name}}) {
			$found = 0;

			if ($sym eq '__uClibc_start_main') {
				$sym = '__uClibc_main';
			}

			#  __gnu_local_gp is defined specially by the linker on MIPS
			if ($sym eq '__gnu_local_gp') {
				$found = 1;
			}
			elsif (resolve($name, $sym) eq "*** unresolved ***") {
				@users = usersOf($name);
				foreach $u (@users) {
					# if exported by $u
					if (defined $elf_exp{$u}{$sym}) {
						fixDynDep($name, $u);
						$found = 1;
					}
					# if exported by shared libs of $u
					if (($t = resolve($u, $sym)) ne "*** unresolved ***") {
						fixDynDep($name, $t);
						$found = 1;
					}
				}
				
				if ($found == 0) {
					print "Unable to resolve $sym used by $name\n", @users;
#					exit 1;
				}
			}
		}
	}
}

sub tab
{
	my $current = shift;
	my $target = shift;
	my $s = "";
	my $n;
	
	while (1) {
		$n = $current + (4 - ($current % 4));
		last if ($n > $target);
		$s = $s . "\t";
		$current = $n;
	}
	while ($current < $target) {
		$s = $s . " ";
		$current++;
	}
	return $s;
}

sub genXref
{
	my $f;
	my $fname;
	my $s;
	my @u;
	
#	print "Generating Xref Report...\n";
	
	open($f, ">libfoo_xref.txt");
	foreach $fname (sort keys %elf_type) {
		print $f "$fname:\n";
		
		if (scalar(@{$elf_lib{$fname}}) > 0) {
			print $f "Dependency:\n";
			foreach $s (sort @{$elf_lib{$fname}}) {
				print $f "\t$s", defined $elf_dyn{$fname}{$s} ? " (dyn)\n" : "\n";
			}
		}
		
		if (scalar(keys %{$elf_exp{$fname}}) > 0) {
			print $f "Export:\n";
			foreach $s (sort keys %{$elf_exp{$fname}}) {
				@u = usersOf($fname, $s);
				if (scalar(@u) > 0) {
					print $f "\t$s", tab(length($s) + 4, 40), " > ", join(",", @u), "\n";
				}
				else {
					print $f "\t$s\n";
				}
			}
		}
		
		if (scalar(keys %{$elf_ext{$fname}}) > 0) {
			print $f "External:\n";
			foreach $s (sort keys %{$elf_ext{$fname}}) {
				print $f "\t$s", tab(length($s) + 4, 40), " < ", resolve($fname, $s), "\n";
			}
		}
		
		print $f "\n";
	}
	close($f);
}


sub genSO
{
	my ($so, $arc, $strip, $opt) = @_;
	my $name = basename($so);
	my $sym;
	my $fn;
	my $inuse;
	my @used;
	my @unused;
	my $cmd;
	my $before, $after;
	my @so_fname;
	my $so_symlinks;

	if (!-f $so) {
		print "$name: not found, skipping...\n";
		return 0;
	}

	#!!TB
	if (!-f $arc) {
		print "$arc: not found, skipping...\n";
		return 0;
	}
	
	foreach $sym (sort keys %{$elf_exp{$name}}) {
		if ((scalar(usersOf($name, $sym)) > 0) || (${strip} eq "no")) {
			push(@used, $sym);
		}
		else {
			push(@unused, $sym);
		}
	}

#	print "\n$name: Attempting to link ", scalar(@used), " and remove ", scalar(@unused), " objects...\n";

	print LOG "\n\n${base}\n";
	
#	$cmd = "mipsel-uclibc-ld -shared -s -z combreloc --warn-common --fatal-warnings ${opt} -soname ${name} -o ${so}";
	$cmd = "mipsel-uclibc-gcc -shared -nostdlib -Wl,-s,-z,combreloc -Wl,--warn-common -Wl,--fatal-warnings -Wl,--gc-sections ${opt} -Wl,-soname=${name} -o ${so}";
	foreach (@{$elf_lib{$name}}) {
		if ((!$elf_dyn{$name}{$_}) && (/^lib(.+)\.so/)) {
			$cmd .= " -l$1";
		}
		else {
#			print LOG "Not marking for linkage: $_\n";
		}
	}
#	print "$cmd -u... ${arc}\n";	
	if (scalar(@used) == 0) {
		print "$name: WARNING: Library is not used by anything, deleting...\n";
		unlink $so;
		@so_fname=split(/\./,$name);
		$so_symlinks=dirname($so)."\/".@so_fname[0]."\.so\*";
		print "$so_symlinks: WARNING: Symbol links is invalid, deleting...\n";
		unlink glob $so_symlinks;
#		<>;
		return 0;
	}
	$cmd .= " -u " . join(" -u ", @used) . " ". $arc;

	print LOG "Command: $cmd\n";
	print LOG "Used: ", join(",", @used), "\n";
	print LOG "Unused: ", join(",", @unused), "\n";
	
	$before = -s $so;

	system($cmd);
	if ($? != 0) {
		error("ld returned $?");
	}

	$after = -s $so;
	
	print "$name: Attempted to remove ", scalar(@unused), "/", scalar(@unused) + scalar(@used), " symbols. ";
	printf "%.2fK - %.2fK = %.2fK\n", $before / 1024, $after / 1024, ($before - $after) / 1024;
	
#	print "\n$name: Attempting to link ", scalar(@used), " and remove ", scalar(@unused), " objects...\n";
#	printf "Before: %.2fK / After: %.2fK / Removed: %.2fK\n\n", $before / 1024, $after / 1024, ($before - $after) / 1024;

	return ($before > $after)
}


##
##
##

#	print "\nlibfoo.pl - fooify shared libraries\n";
#	print "Copyright (C) 2006-2007 Jonathan Zarate\n\n";

if ((!-d $root) || (!-d $uclibc) || (!-d $router)) {
	print "Missing or invalid environment variables\n";
	exit(1);
}

open(LOG, ">libfoo.debug");
#open(LOG, ">/dev/null");

print "Loading...\r";
load($root);
print "Finished loading files.", " " x 30, "\r";

fixDyn();
fillGaps();

genXref();

$stripshared = "yes";
if ($ARGV[0] eq "--noopt") {
	$stripshared = "no";
}

genSO("${root}/lib/libc.so.0", "${uclibc}/lib/libc.a", "", "-Wl,-init=__uClibc_init ${uclibc}/lib/optinfo/interp.os");
genSO("${root}/lib/libresolv.so.0", "${uclibc}/lib/libresolv.a", "${stripshared}");
genSO("${root}/lib/libcrypt.so.0", "${uclibc}/lib/libcrypt.a", "${stripshared}");
genSO("${root}/lib/libm.so.0", "${uclibc}/lib/libm.a");
genSO("${root}/lib/libpthread.so.0", "${uclibc}/lib/libpthread.a", "${stripshared}", "-u pthread_mutexattr_init -Wl,-init=__pthread_initialize_minimal_internal");
genSO("${root}/lib/libutil.so.0", "${uclibc}/lib/libutil.a", "${stripshared}");
#  genSO("${root}/lib/libdl.so.0", "${uclibc}/lib/libdl.a", "${stripshared}");
#  genSO("${root}/lib/libnsl.so.0", "${uclibc}/lib/libnsl.a", "${stripshared}");

genSO("${root}/usr/lib/libcrypto.so.1.0.0", "${router}/openssl/libcrypto.a");
genSO("${root}/usr/lib/libssl.so.1.0.0", "${router}/openssl/libssl.a", "${stripshared}", "-L${router}/openssl");

genSO("${root}/usr/lib/libzebra.so", "${router}/zebra/lib/libzebra.a");
genSO("${root}/usr/lib/libz.so.1", "${router}/zlib/libz.a");
genSO("${root}/usr/lib/libjpeg.so", "${router}/jpeg/libjpeg.a");
genSO("${root}/usr/lib/libsqlite3.so.0", "${router}/sqlite/.libs/libsqlite3.a");
genSO("${root}/usr/lib/libogg.so.0", "${router}/libogg/src/.libs/libogg.a");
genSO("${root}/usr/lib/libvorbis.so.0", "${router}/libvorbis/lib/.libs/libvorbis.a", "", "-L${router}/libogg/src/.libs");
genSO("${root}/usr/lib/libid3tag.so.0", "${router}/libid3tag/.libs/libid3tag.a", "", "-L${router}/zlib");
genSO("${root}/usr/lib/libexif.so.12", "${router}/libexif/libexif/.libs/libexif.a");
genSO("${root}/usr/lib/libFLAC.so.8", "${router}/flac/src/libFLAC/.libs/libFLAC.a", "", "-L${router}/libogg/src/.libs");
genSO("${root}/usr/lib/libavcodec.so.52", "${router}/ffmpeg/libavcodec/libavcodec.a", "", "-L${router}/ffmpeg/libavutil -L${router}/zlib");
genSO("${root}/usr/lib/libavutil.so.50", "${router}/ffmpeg/libavutil/libavutil.a", "-L${router}/zlib");
genSO("${root}/usr/lib/libavformat.so.52", "${router}/ffmpeg/libavformat/libavformat.a", "", "-L${router}/ffmpeg/libavutil -L${router}/ffmpeg/libavcodec -L${router}/zlib");
genSO("${root}/usr/lib/libsmb.so", "${router}/samba/source/bin/libsmb.a");
genSO("${root}/usr/lib/libbigballofmud.so", "${router}/samba3/source/bin/libbigballofmud.a");

genSO("${root}/usr/lib/liblzo2.so.2", "${router}/lzo/src/.libs/liblzo2.a");
#	genSO("${root}/usr/lib/libtamba.so", "${router}/samba3/source/bin/libtamba.a");
#	genSO("${root}/usr/lib/libiptc.so", "${router}/iptables/libiptc/libiptc.a");
#	genSO("${root}/usr/lib/libshared.so", "${router}/shared/libshared.a");
#	genSO("${root}/usr/lib/libnvram.so", "${router}/nvram/libnvram.a");
#	genSO("${root}/usr/lib/libusb-1.0.so.0", "${router}/libusb10/libusb/.libs/libusb-1.0.a");
#	genSO("${root}/usr/lib/libusb-0.1.so.4", "${router}/libusb/libusb/.libs/libusb.a", "", "-L${router}/libusb10/libusb/.libs");

genSO("${root}/usr/lib/libbcmcrypto.so", "${router}/libbcmcrypto/libbcmcrypto.a");

#shibby
genSO("${root}/usr/lib/libcurl.so.4.3.0", "${router}/libcurl/lib/.libs/libcurl.a", "", "-L${router}/zlib");
genSO("${root}/usr/lib/libevent-2.0.so.5", "${router}/libevent/.libs/libevent.a");
genSO("${root}/usr/lib/libdaemon.so.0.5.0", "${router}/libdaemon/libdaemon/.libs/libdaemon.a");
#genSO("${root}/usr/lib/libiconv.so.2", "${router}/libiconv/lib/.libs/libiconv.a");
genSO("${root}/usr/lib/libiconv.so.2.4.0", "${router}/libiconv/lib/.libs/libiconv.a");
genSO("${root}/usr/lib/libnfnetlink.so.0.2.0", "${router}/libnfnetlink/src/.libs/libnfnetlink.a");
genSO("${root}/usr/lib/libsodium.so.4.5.0", "${router}/libsodium/src/libsodium/.libs/libsodium.a");
#bwq
#genSO("${root}/usr/lib/libexpat.so.1.6.0", "${router}/libexpat/.libs/libexpat.a");
#genSO("${root}/usr/lib/libcares.so.2.0.0", "${router}/libcares/.libs/libcares.a");
#genSO("${root}/usr/lib/libpython2.7.so.1.0", "${router}/python/libpython2.7.a");
#genSO("${root}/usr/lib/libncurses.so.5", "${router}/libncurses/lib/libncurses.a");
#genSO("${root}/usr/lib/libdb-4.8.so", "${router}/libdb/build_unix/.libs/libdb-4.8.a");
#genSO("${root}/usr/lib/libpanel.so.5", "${router}/libncurses/lib/libpanel.a");

print "\n";

close(LOG);
exit(0);
