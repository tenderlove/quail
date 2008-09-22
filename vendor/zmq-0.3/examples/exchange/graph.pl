#!/usr/bin/perl -w
#
#   Copyright (c) 2007-2008 FastMQ Inc.
#
#   This file is part of 0MQ.
#
#   0MQ is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   0MQ is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#   graph.pl: Graph output from 'stat' program.
#   Requires Perl-Tk installed (package perl-tk on Debian), and 
#   Tk::Graph from CPAN.
#
#   Usage example:
#
#       $ ./stat LOCATOR_HOST LOCATOR_PORT INTERFACE | ./graph.pl
use strict;
use Fcntl;
use English;
use Tk;
use Tk::Graph;

my (%lat_meters, %thr_meters);
my ($window, $poll_interval, $flags);

$flags = O_NONBLOCK;
fcntl (STDIN, F_SETFL, $flags)
    or die "Couldn't set flags for STDIN: $!\n";

$poll_interval = 1;
%thr_meters = ('1' => 0, '2' => 0, '3' => 0, '4' => 0);
%lat_meters = ('l' => 0);

$window = Tk::MainWindow->new (
    -title => "Throughput and latency statistics");
$window->geometry ("1024x768");

&draw_graph ($window);
Tk::MainLoop;

exit 0;

# -----------------------------------------------------------------------------

sub draw_graph {
    my ($target) = @_;
    my ($frame, $thr_graph, $lat_graph);

    $frame = $target->Frame()->pack (
        -side   => 'left',
        -fill   => 'both',
        -expand => 1);
    $thr_graph = $frame->Graph (
        -title       => 'Throughput',
        -background  => 'black',
        -type        => 'Line',
        -legend      => 0,
        -headroom    => 20,
        -foreground  => 'green',
        -debug       => 0,
        -borderwidth => 2,
        -titlecolor  => 'green',
        -yformat     => '%llu',
        -ylabel      => 'msgs/s',
        -xformat     => '%d',
        -xlabel      => 't',
        -barwidth    => 15,
        -padding     => [50, 20, -30, 70],
        -printvalue  => '%s',
        -linewidth   => 3,
        -dots        => 1,
        -look        => 20, 
        -wire        => "#2c5c1d",
        -max	     => 3000000,
        -ytick       => 10,
        -xtick       => 20,
        -config      => { 
                        '1'  => { -color => 'orange' },
                        '2'  => { -color => 'red' },
                        '3', => { -color => 'red' }, 
                        '4', => { -color => 'orange' },
                        },
	);
    $lat_graph = $frame->Graph (
        -title       => 'Latency',
        -background  => 'black',
        -type        => 'Line',
        -legend      => 0,
        -headroom    => 20,
        -foreground  => 'green',
        -debug       => 0,
        -borderwidth => 2,
        -titlecolor  => 'green',
        -yformat     => '%llu',
        -ylabel      => 'usecs',
        -xformat     => '%d',
        -xlabel      => 't',
        -barwidth    => 15,
        -padding     => [50, 20, -30, 70],
        -printvalue  => '%s',
        -linewidth   => 3,
        -dots        => 1,
        -look        => 20, 
        -wire        => "#2c5c1d",
        -max	     => 5000,
        -ytick       => 10,
        -xtick       => 20,
        -config      => { 
                        'l', => { -color => 'yellow' }
                        },
	);
    &update_data ($thr_graph, $lat_graph, $poll_interval * 1000);
    $thr_graph->pack (
        -side   => 'bottom',
        -expand => 1,
        -fill   => 'both',
	);
    return $lat_graph->pack (
        -side   => 'bottom',
        -expand => 1,
        -fill   => 'both',
	);
}

# -----------------------------------------------------------------------------

sub update_data {
    my ($thr_graph, $lat_graph, $poll_msec) = @_;
    my ($data, $rv, $meter, $value);

    while (1) {
        $rv = sysread (STDIN, $data, 11);
        last if (!defined ($rv));
        chomp ($data);
        my ($meter, $value) = split /:/, $data;
        if ($meter eq 'l') {
            $lat_meters{$meter} = int ($value);
        } else {
            $thr_meters{$meter} = int ($value);
        }
    }
    print "1=", $thr_meters{'1'}, " 2=", $thr_meters{'2'}, 
         " 3=", $thr_meters{'3'}, " 4=", $thr_meters{'4'}, 
         " l=", $lat_meters{'l'}, "\n";

    $thr_graph->set (\%thr_meters);
    $lat_graph->set (\%lat_meters);
    $thr_graph->after ($poll_msec, 
        [\&update_data => $thr_graph, $lat_graph, $poll_msec]);
}

