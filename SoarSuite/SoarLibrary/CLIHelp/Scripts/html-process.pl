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
  $element->delete();
}

while (my $element = $html->find("img")) {
  $element->delete();
}

while (my $element = $html->find("meta")) {
  $element->delete();
}

while (my $element = $html->find("link")) {
  $element->delete();
}

while (my $element = $html->find("script")) {
  $element->delete();
}

while (my $element = $html->look_down("id","siteSub")) {
  $element->delete();
}

while (my $element = $html->look_down("id","column-one")) {
  $element->delete();
}

while (my $element = $html->look_down("id","footer")) {
  $element->delete();
}

while (my $element = $html->look_down("class","printfooter")) {
  $element->delete();
}

while (my $element = $html->look_down("id","toc")) {
  $element->delete();
}

while (my $element = $html->look_down("id","catlinks")) {
  $element->delete();
}

while (my $element = $html->look_down("class","editsection")) {
  $element->delete();
}

while (my $element = $html->look_down("name","Structured_Output")) {
  my $parent = $element->parent();
  #$parent->dump();
  #exit;
  my @contents = $parent->content_list();
  my $foundit = 0;
  
  foreach (@contents) {
    my @attrs = $_->all_external_attr();
    if (@attrs) {
      if ($attrs[0] eq "name") {
        if ($attrs[1] eq "Structured_Output") {
	  $foundit = 1;
	}
      }
    }
    if ($foundit == 1) {
      $_->delete();
    }
  }
}

print $html->as_HTML;
#$html->dump;
