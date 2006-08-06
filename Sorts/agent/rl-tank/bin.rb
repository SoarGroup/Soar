#!/usr/bin/ruby

bin_def_pattern = /((\[|\()\d+,\d+(\]|\)):\w+;)*(\(|\[)\d+,\d+(\]|\)):\w+/
#                             1    2     3     4      5
single_bin_def_pattern = /(\[|\()(\d+),(\d+)(\]|\)):(\w+)/

header = 
<<EOS
########################################################################
#                                                                      #
#               This file was generated automatically                  #
#                                                                      #
########################################################################
EOS

bin_def = ""
op_name = ""
soar_file_name = ""
line_count = 1
input = ""



if ARGV.length == 0
  source = STDIN
else
  source = File.new(ARGV[0], "r")
end

source.each { |line|
  if line_count == 1
    op_name = line.strip
    line_count += 1
  elsif line_count == 2
    bin_def = line.delete(" ").strip
    if not bin_def =~ bin_def_pattern
      puts "Binning rules incorrect"
      exit(1)
    end
    line_count += 1
  elsif line_count == 3
    soar_file_name = line.strip
    line_count += 1
  else
    input = input + line
  end
}

if soar_file_name == "stdout"
  soar_file = $stdout
else
  soar_file = File.new(soar_file_name, "w")
  soar_file << header << "\n"
end

while true
  m = bin_def.match(single_bin_def_pattern)

  left_inclusive = (m[1] == "[")
  right_inclusive = (m[4] == "]")

  if left_inclusive and right_inclusive
    if (m[2] == m[3])
      int_cond = m[2]
    else
      int_cond = "{ >= #{m[2]} <= #{m[3]} }"
    end
  elsif left_inclusive
    int_cond = "{ >= #{m[2]} < #{m[3]} }"
  elsif right_inclusive
    int_cond = "{ > #{m[2]} <= #{m[3]} }"
  else
    int_cond = "{ > #{m[2]} < #{m[3]} }"
  end

  op_instance_name = op_name + "-" + m[5]
 
  input_instance = input[0..input.length]
  input_instance.gsub!(/\\int\\/, int_cond)
  input_instance.gsub!(/\\bin\\/, m[5])
  input_instance.gsub!(/\\op_name\\/, op_instance_name)

  soar_file << input_instance 

  bin_def = bin_def[m.end(0)+1..bin_def.length]

  if (bin_def == nil) 
    break
  end
end
