#!/usr/bin/perl

use strict;

use HTML::TreeBuilder;

my $tb = HTML::TreeBuilder->new;
$tb->ignore_text(0);
$tb->ignore_unknown(1);
$tb->implicit_tags(1);
$tb->warn(1);

my $html = $tb->parse_file($ARGV[0]);

while (my $element = $html->find("style")) {
  #print "SERVED a style\n";
  $element->delete();
}

while (my $element = $html->find("img")) {
  #print "SERVED an img\n";
  $element->delete();
}

while (my $element = $html->find("meta")) {
  #print "SERVED a meta\n";
  $element->delete();
}

while (my $element = $html->find("link")) {
  #print "SERVED a link\n";
  $element->delete();
}

while (my $element = $html->find("script")) {
  #print "SERVED a script\n";
  $element->delete();
}

while (my $element = $html->look_down("id","column-one")) {
  #print "SERVED column-one\n";
  $element->delete();
}

while (my $element = $html->look_down("id","footer")) {
  #print "SERVED footer\n";
  $element->delete();
}

while (my $element = $html->look_down("id","toc")) {
  #print "SERVED toc\n";
  $element->delete();
}

while (my $element = $html->look_down("class","editsection")) {
  #print "SERVED editsection\n";
  $element->delete();
}

print $html->as_HTML;
#$html->dump;
