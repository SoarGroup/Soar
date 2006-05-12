#!/usr/bin/ruby

# Parses Soar working memory syntax and outputs a graphviz representation.
#
# Joseph Xu
# April 13, 2006

# A general parenthesis parser, a little overkill for this application
class Block
  attr_reader :contents, :enclosingType, :blocksize, :children

  def initialize(contents, enclosingType)
    @contents = contents
    @enclosingType = enclosingType
    @children = Array.new
    @blocksize = contents.size
  end

  def parse(line)
    while line.size > 0
      n = line.index(/\{|\[|\(|\)|\]|\}/)
      if n == nil # end of the line
        if @enclosingType != 0
          # missing parens
        end
        @children.push(Block.new(line[0..-1], "")) if line.size > 0
        @blocksize += line.size
        return
      elsif line[n..n] =~ /\{|\(|\[/ # nesting
        if n > 0
          @children.push(Block.new(line[0..n-1], ""))
          @blocksize += n
        end
        child = Block.new("", line[n..n])
        @children.push(child)
        child.parse(line[n+1..-1])
        @blocksize += child.blocksize + 2 # +2 for open and close paren
        line = line[n + child.blocksize + 2..-1]
      else # unnesting
        if (@enclosingType == "(" && line[n..n] != ")") ||
           (@enclosingType == "[" && line[n..n] != "]") ||
           (@enclosingType == "{" && line[n..n] != "}")
          puts "paren mismatch, got " + line[n..n]
          Kernel.exit(1) 
        end
        @children.push(Block.new(line[0..n-1], "")) if n > 0
        @blocksize += n
        return
      end
    end
  end

  def print(indent)
    if @contents.size > 0
      puts " " * indent + @contents
    else
      @children.each { |c| c.print(indent+2) }
    end
  end
end

class Parser
  def initialize
    @nodename = "a"
    @identifiers = Hash.new(0)
  end

  def parseblock(b)
    tokens = b.split
    if @identifiers.has_key?(tokens[0])
      curr_node_name = @identifiers[tokens[0]]
    else
      curr_node_name = @nodename
      @identifiers[tokens[0]] = curr_node_name
      @nodename = @nodename.next
    end

    i = 1
    while i < tokens.size
      if !(tokens[i] =~ /\^\w+/)
        puts "unexpected token " + tokens[i]
        Kernel.exit(1)
      end
      if tokens[i+1] =~ /\A[a-zA-Z]\d+/
        # this is an identifier
        if @identifiers.has_key?(tokens[i+1])
          puts "#{curr_node_name} -> #{@identifiers[tokens[i+1]]} [label=\"#{tokens[i]}\"]"
        else
          # reserve this for later
          @identifiers[tokens[i+1]] = @nodename
          puts "#{curr_node_name} -> #{@nodename} [label=\"#{tokens[i]}\"]"
          @nodename = @nodename.next
        end
      else
        puts "#{curr_node_name} -> #{@nodename} [label=\"#{tokens[i]}\"]"
        puts "#{@nodename} [label=\"#{tokens[i+1]}\"]"
        @nodename = @nodename.next
      end
      i += 2
    end
  end

  def print_node_attribs
    @identifiers.each_pair {|key, val| puts "#{val} [label=\"#{key}\"]"}
  end

  def doEverything(input)
    puts "digraph G {"

    blocks = Block.new("", "")
    blocks.parse(input)
    blocks.children.each {|c| c.children.each {|c1| parseblock(c1.contents) } }

    print_node_attribs

    puts "}"
  end
end

input = ""
ARGF.each { |line| input.concat(line) }
p = Parser.new
p.doEverything(input)
