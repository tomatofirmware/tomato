#!/usr/bin/perl
#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 2011 - 2013, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://curl.haxx.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
###########################################################################

my $max_column = 79;
my $indent = 2;

my $warnings;
my $errors;
my $file;
my $dir=".";
my $wlist;
<<<<<<< HEAD
=======
my $windows_os = $^O eq 'MSWin32' || $^O eq 'msys' || $^O eq 'cygwin';
my $verbose;
my %whitelist;

my %warnings = (
    'LONGLINE' =>         "Line longer than $max_column",
    'TABS' =>             'TAB characters not allowed',
    'TRAILINGSPACE' =>    'Trailing white space on the line',
    'CPPCOMMENTS' =>      '// comment detected',
    'SPACEBEFOREPAREN' => 'space before an open parenthesis',
    'SPACEAFTERPAREN'  => 'space after open parenthesis',
    'SPACEBEFORECLOSE' => 'space before a close parenthesis',
    'SPACEBEFORECOMMA' => 'space before a comma',
    'RETURNNOSPACE'    => 'return without space',
    'COMMANOSPACE'     => 'comma without following space',
    'BRACEELSE'        => '} else on the same line',
    'PARENBRACE'       => '){ without sufficient space',
    'SPACESEMILCOLON'  => 'space before semicolon',
    'BANNEDFUNC'       => 'a banned function was used',
    'FOPENMODE'        => 'fopen needs a macro for the mode string',
    'BRACEPOS'         => 'wrong position for an open brace',
    'INDENTATION'      => 'wrong start column for code',
    'COPYRIGHT'        => 'file missing a copyright statement',
    'BADCOMMAND'       => 'bad !checksrc! instruction',
    'UNUSEDIGNORE'     => 'a warning ignore was not used',
    'OPENCOMMENT'      => 'file ended with a /* comment still "open"',
    'ASTERISKSPACE'    => 'pointer declared with space after asterisk',
    'ASTERISKNOSPACE'  => 'pointer declared without space before asterisk',
    'ASSIGNWITHINCONDITION'  => 'assignment within conditional expression'
    );

sub readwhitelist {
    open(W, "<$dir/checksrc.whitelist");
    my @all=<W>;
    for(@all) {
        $windows_os ? $_ =~ s/\r?\n$// : chomp;
        $whitelist{$_}=1;
    }
    close(W);
}
>>>>>>> origin/tomato-shibby-RT-AC

sub checkwarn {
    my ($num, $col, $file, $line, $msg, $error) = @_;

    my $w=$error?"error":"warning";

    if($w) {
        $warnings++;
    }
    else {
        $errors++;
    }

    $col++;
    print "$file:$num:$col: $w: $msg\n";
    print " $line\n";

    if($col < 80) {
        my $pref = (' ' x $col);
        print "${pref}^\n";
    }
}

$file = shift @ARGV;

while(1) {

    if($file =~ /-D(.*)/) {
        $dir = $1;
        $file = shift @ARGV;
        next;
    }
    elsif($file =~ /-W(.*)/) {
        $wlist .= " $1 ";
        $file = shift @ARGV;
        next;
    }

    last;
}

if(!$file) {
    print "checksrc.pl [option] <file1> [file2] ...\n";
    print " Options:\n";
    print "  -D[DIR]   Directory to prepend file names\n";
    print "  -W[file]  Whitelist the given file - ignore all its flaws\n";
    exit;
}

do {
    if("$wlist" !~ / $file /) {
        my $fullname = $file;
        $fullname = "$dir/$file" if ($fullname !~ '^\.?\.?/');
        scanfile($fullname);
    }
    $file = shift @ARGV;

} while($file);


sub nostrings {
    my ($str) = @_;
    $str =~ s/\".*\"//g;
    return $str;
}

sub scanfile {
    my ($file) = @_;

    my $line = 1;
    my $prevl;
    my $l;
    open(R, "<$file") || die "failed to open $file";

    my $copyright=0;

    while(<R>) {
        chomp;
        my $l = $_;
        my $column = 0;

        # check for a copyright statement
        if(!$copyright && ($l =~ /copyright .* \d\d\d\d/i)) {
            $copyright=1;
        }

        # detect long lines
        if(length($l) > $max_column) {
            checkwarn($line, length($l), $file, $l, "Longer than $max_column columns");
        }
        # detect TAB characters
        if($l =~ /^(.*)\t/) {
            checkwarn($line, length($1), $file, $l, "Contains TAB character", 1);
        }
        # detect trailing white space
        if($l =~ /^(.*)[ \t]+\z/) {
            checkwarn($line, length($1), $file, $l, "Trailing whitespace");
        }

        my $nostr = nostrings($l);
        # check spaces after for/if/while/function call
        if($nostr =~ /^(.*)(for|if|while| ([a-zA-Z0-9_]+)) \((.)/) {
            if($1 =~ / *\#/) {
                # this is a #if, treat it differently
            }
            elsif($3 eq "return") {
                # return must have a space
            }
            elsif($4 eq "*") {
                # (* beginning makes the space OK!
            }
            elsif($1 =~ / *typedef/) {
                # typedefs can use space-paren
            }
            else {
                checkwarn($line, length($1)+length($2), $file, $l,
                          "$2 with space");
            }
        }

<<<<<<< HEAD
        # check spaces after open paren after for/if/while
        if($l =~ /^(.*)(for|if|while)\( /) {
=======
        if($nostr =~ /^((.*)(if) *\()(.*)\)/) {
            my $pos = length($1);
            if($4 =~ / = /) {
                checkwarn("ASSIGNWITHINCONDITION",
                          $line, $pos+1, $file, $l,
                          "assignment within conditional expression");
            }
        }
        # check spaces after open parentheses
        if($l =~ /^(.*[a-z])\( /i) {
            checkwarn("SPACEAFTERPAREN",
                      $line, length($1)+1, $file, $l,
                      "space after open parenthesis");
        }

        # check spaces before close parentheses, unless it was a space or a
        # close parenthesis!
        if($l =~ /(.*[^\) ]) \)/) {
            checkwarn("SPACEBEFORECLOSE",
                      $line, length($1)+1, $file, $l,
                      "space before close parenthesis");
        }

        # check spaces before comma!
        if($l =~ /(.*[^ ]) ,/) {
            checkwarn("SPACEBEFORECOMMA",
                      $line, length($1)+1, $file, $l,
                      "space before comma");
        }

        # check for "return(" without space
        if($l =~ /^(.*)return\(/) {
>>>>>>> origin/tomato-shibby-RT-AC
            if($1 =~ / *\#/) {
                # this is a #if, treat it differently
            }
            else {
                checkwarn($line, length($1)+length($2)+1, $file, $l,
                          "$2 with space first in condition");
            }
        }

        # check for "} else"
        if($l =~ /^(.*)\} *else/) {
            checkwarn($line, length($1), $file, $l, "else after closing brace on same line");
        }
        # check for "){"
        if($l =~ /^(.*)\)\{/) {
            checkwarn($line, length($1)+1, $file, $l, "missing space after close paren");
        }

        # scan for use of banned functions
<<<<<<< HEAD
        if($l =~ /^(.*\W)(sprintf|vsprintf|strcat|strncat|gets)\s*\(/) {
            checkwarn($line, length($1), $file, $l,
=======
        if($l =~ /^(.*\W)
                   (gets|
	            strtok|
                    v?sprintf|
                    (str|_mbs|_tcs|_wcs)n?cat|
                    LoadLibrary(Ex)?(A|W)?)
                   \s*\(
                 /x) {
            checkwarn("BANNEDFUNC",
                      $line, length($1), $file, $ol,
>>>>>>> origin/tomato-shibby-RT-AC
                      "use of $2 is banned");
        }

        # check for open brace first on line but not first column
        # only alert if previous line ended with a close paren and wasn't a cpp
        # line
        if((($prevl =~ /\)\z/) && ($prevl !~ /^ *#/)) && ($l =~ /^( +)\{/)) {
            checkwarn($line, length($1), $file, $l, "badly placed open brace");
        }

        # if the previous line starts with if/while/for AND ends with an open
        # brace, check that this line is indented $indent more steps, if not
        # a cpp line
        if($prevl =~ /^( *)(if|while|for)\(.*\{\z/) {
            my $first = length($1);

            # this line has some character besides spaces
            if(($l !~ /^ *#/) && ($l =~ /^( *)[^ ]/)) {
                my $second = length($1);
                my $expect = $first+$indent;
                if($expect != $second) {
                    my $diff = $second - $first;
                    checkwarn($line, length($1), $file, $l,
                              "not indented $indent steps, uses $diff)");

                }
            }
        }

        # check for 'char * name'
        if(($l =~ /(^.*(char|int|long|void|curl_slist|CURL|CURLM|CURLMsg|curl_httppost) *(\*+)) (\w+)/) && ($4 ne "const")) {
            checkwarn("ASTERISKNOSPACE",
                      $line, length($1), $file, $ol,
                      "no space after declarative asterisk");
        }
        # check for 'char*'
        if(($l =~ /(^.*(char|int|long|void|curl_slist|CURL|CURLM|CURLMsg|curl_httppost|sockaddr_in|FILE)\*)/)) {
            checkwarn("ASTERISKNOSPACE",
                      $line, length($1)-1, $file, $ol,
                      "no space before asterisk");
        }

        # check for 'void func() {', but avoid false positives by requiring
        # both an open and closed parentheses before the open brace
        if($l =~ /^((\w).*){\z/) {
            my $k = $1;
            $k =~ s/const *//;
            $k =~ s/static *//;
            if($k =~ /\(.*\)/) {
                checkwarn("BRACEPOS",
                          $line, length($l)-1, $file, $ol,
                          "wrongly placed open brace");
            }
        }
        $line++;
        $prevl = $l;
    }

    if(!$copyright) {
        checkwarn(1, 0, $file, "", "Missing copyright statement", 1);
    }

    close(R);

}


if($errors || $warnings) {
    printf "checksrc: %d errors and %d warnings\n", $errors, $warnings;
    exit 5; # return failure
}
