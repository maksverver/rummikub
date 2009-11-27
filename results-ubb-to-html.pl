#!/usr/bin/env perl

print <<EOF;
<html>
<head>
  <title>Resultaten</title>
  <link rel="stylesheet" type="text/css" href="transcript.css">
</head>
<body style="background:#e0e0e0; color:black;">
<h1>Resultaten</h1>
EOF

while (<>) {
	
	for (m/\[[^]]+]|[^[]+/g) {

		if (/^\[\/(.*)]$/) {  # UBB close tag

			$tag = pop(@tags);
			$_ = "</$tag>";

		} elsif (($path,$res) = /^\[url=(.*)\/(.*)]$/) {  # UBB url

			push(@tags, 'a');
			$_ = "<a href=\"$res\">";

		} elsif (($color) = /^\[(red|green|blue)]$/) {  # UBB color

			push(@tags, 'span');
			$_ = "<span style=\"color:$color\">";


		} elsif (($tag, $args) = /^\[([^\s]+)(.*)]$/) {  # UBB open tag

			# quote attribute values:
			$args =~ s/([^\s]+)=([^\s]+)/$1="$2"/g;

			# expand color codes to 6 hex digits:
			$args =~ s/"#([0-9a-f])([0-9a-f])([0-9a-f])"/"#$1$1$2$2$3$3"/g; 

			push(@tags, $tag);
			$_ = "<$tag$args>";
		}

		print;
	}
}

print <<EOF;
</body>
</html>
EOF
