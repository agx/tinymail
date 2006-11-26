#! /usr/bin/perl

my $typename = "MyType";
# my %methods = ();
my %interfaces = ();
my $tel=0;
my $itel=0;

sub max     { $_[0] > $_[1] ? $_[0] : $_[1]; }
sub isupper { ord($_[0]) >= ord('A') && ord($_[0]) <= ord('Z'); }
sub islower { ord($_[0]) >= ord('a') && ord($_[0]) <= ord('z'); }
sub toupper { &islower ? pack('c', ord($_[0])-ord('a')+ord('A')) : $_[0];}
sub tolower { &isupper ? pack('c', ord($_[0])-ord('A')+ord('a')) : $_[0];}

sub uncamel_low
{
	my $line = shift;
	my $char;
	my $c;
	my $out;
	my $i=0;
	
	for ($i=0; $i < length ($line); $i++)
	{
		$char = substr ($line, $i, 1);

		if ($char =~ /[A-Z]/) {
			if ($i == 0) {
				$c = "";
			} else {
				$c = "_";
			}
			$c .= tolower (substr ($char, 0, 1));
			$out .= $c;
		} else {
			$out .= substr ($char, 0, 1);
		}
	}

	return $out;
}

sub uncamel_file
{
	my $line = shift;
	my $char;
	my $c;
	my $out;
	my $i=0;	
	
	for ($i=0; $i < length ($line); $i++)
	{
		$char = substr ($line, $i, 1);

		if ($char =~ /[A-Z]/) {
			if ($i == 0) {
				$c = "";
			} else {
				$c = "-";
			}
			$c .= tolower (substr ($char, 0, 1));
			$out .= $c;
		} else {
			$out .= substr ($char, 0, 1);
		}
	}

	return $out;
}
sub uncamel_up
{
	my $line = shift;
	my $char;
	my $c;
	my $out;
	my $i=0;
	
	for ($i=0; $i < length ($line); $i++)
	{
		$char = substr ($line, $i, 1);

		if ($char =~ /[A-Z]/) {
			if ($i == 0) {
				$c = "";
			} else {
				$c = "_";
			}
			$c .= $char;
			$out .= $c;
		} else {
			$out .= toupper (substr ($char, 0, 1));
		}
	}

	return $out;
}

sub unpointer
{
	my $line = shift;

	$line =~ s/\*//g;

	return $line;
}

my $ifaisopen = 0;

while ($line = <STDIN>)
{

	if ($line =~ /struct\s_(.*)Iface[\s|]$/)
	{
		$itel++;
		$interfaces{$itel}{"name"} = $1;
		$ifaisopen = $itel;
		$tel = 0;
	}

	if ($line =~ /^\]\;$/)
	{
		$ifaisopen = 0;
	}

	if ($ifaisopen != 0)
	{
		if ($line =~ /^\s(.*)[\s|]\(\*(.*)_func\)\s(.*)\;$/)
		{
			$interfaces{$ifaisopen}{"methods"}{$tel}{"return_type"} = $1;
			$interfaces{$ifaisopen}{"methods"}{$tel}{"func_name"} = $2;
			$interfaces{$ifaisopen}{"methods"}{$tel}{"params"} = $3;
			$tel++;
		}
	}

}


print ("/* Your copyright here*/\n\n");
print ("#include <config.h>\n");
print ("#include <glib.h>\n");
print ("#include <glib/gi18n-lib.h>\n\n");
print ("#include <".uncamel_file ($typename).".h>\n");
print ("#include \"".uncamel_file ($typename)."-priv.h\"\n\n");
print ("static GObjectClass *parent_class = NULL;\n\n");

for my $key (keys %interfaces)
{
  for my $mkey (keys % { $interfaces{$key}{"methods"} } )
  {
	print ("static ");
	print ($interfaces{$key}{"methods"}{$mkey}{"return_type"}."\n");
	print (uncamel_low($typename)."_");
	print ($interfaces{$key}{"methods"}{$mkey}{"func_name"}." ");
	print ($interfaces{$key}{"methods"}{$mkey}{"params"});
	print ("\n{\n");
	if ($interfaces{$key}{"methods"}{$mkey}{"return_type"} eq "void")
	{
		print ("\treturn;");
	} else {
		print ("\treturn ".uncamel_low(unpointer($interfaces{$key}{"methods"}{$mkey}{"return_type"}))."_new ()");
	}
	print ("\n}\n\n");
  }
}

print ("static void\n");
print (uncamel_low($typename)."_finalize (GObject *object)\n");
print ("{\n\t(*parent_class->finalize) (object);\n\treturn;\n}\n");

for my $key (keys(%interfaces))
{
	print ("\nstatic void\n");
	print (uncamel_low ($interfaces{$key}{"name"})."_init (".$interfaces{$key}{"name"}."Iface *klass)\n{\n");


	for my $mkey (keys % { $interfaces{$key}{"methods"} } )
  	{
		print ("\tklass->".$interfaces{$key}{"methods"}{$mkey}{"func_name"}."_func = ");
		print (uncamel_low($typename)."_");
		print ($interfaces{$key}{"methods"}{$mkey}{"func_name"}.";\n");
	}

	print ("\n\treturn;\n}\n\n");

}

print ("static void\n");
print (uncamel_low ($typename)."_class_init (".$typename."Class *klass)\n{\n");
print ("\tGObjectClass *object_class;\n\n");
print ("\tparent_class = g_type_class_peek_parent (klass);\n");
print ("\tobject_class = (GObjectClass*) klass;\n");
print ("\tobject_class->finalize = ".uncamel_low ($typename)."_finalize;\n\n");
print ("\treturn;\n}\n");

print ("GType\n");
print (uncamel_low ($typename)."_get_type (void)\n{\n");
print ("\tstatic GType type = 0;\n");
print ("\tif (G_UNLIKELY(type == 0))\n\t{\n");
print ("\t\tstatic const GTypeInfo info = \n\t\t{\n");
print ("\t\t\tsizeof (".$typename."Class),\n");
print ("\t\t\tNULL,   /* base_init */\n");
print ("\t\t\tNULL,   /* base_finalize */\n");
print ("\t\t\t(GClassInitFunc) ".uncamel_low ($typename)."_class_init,   /* class_init */\n");
print ("\t\t\tNULL,   /* class_finalize */\n");
print ("\t\t\tNULL,   /* class_data */\n");
print ("\t\t\tsizeof (".$typename."),\n");
print ("\t\t\t0,      /* n_preallocs */\n");
print ("\t\t\t".uncamel_low ($typename)."_instance_init,    /* instance_init */\n");
print ("\t\t\tNULL\n\t\t};\n\n\n");

for my $key (keys(%interfaces))
{
	print ("\t\tstatic const GInterfaceInfo ".uncamel_low ($interfaces{$key}{"name"})."_info = \n\t\t{\n");
	print ("\t\t\t(GInterfaceInitFunc) ".uncamel_low ($interfaces{$key}{"name"})."_init, /* interface_init */\n");
	print ("\t\t\tNULL,         /* interface_finalize */\n");
	print ("\t\t\tNULL          /* interface_data */\n\t\t}\n\n");
}


print ("\t\ttype = g_type_register_static (G_TYPE_OBJECT,\n");
print ("\t\t\t\".$typename.\",\n");
print ("\t\t\t&info, 0);\n\n");

for my $key (keys(%interfaces))
{
	print ("\t\tg_type_add_interface_static (type, ".uncamel_up ($interfaces{$key}{"name"}).",\n"); 
	print ("\t\t\t&".uncamel_low ($interfaces{$key}{"name"})."_info\n\n");
}
print ("\t}\n\treturn type;\n}\n");

