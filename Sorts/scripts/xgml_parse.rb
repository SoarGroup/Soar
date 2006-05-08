require 'rexml/document'
require 'set'
include REXML

class Task
  attr_reader :name, :instance_of, :params
  attr_writer :params
  def initialize(name, instance_of)
    @name = name
    @instance_of = instance_of
    @params = Hash.new(0)
  end
end

input=""
ARGF.each {|line| input.concat(line)}

tasks = Hash.new(0)
retrievals = Hash.new(0)
deps = Hash.new(0)

doc = Document.new(input)
graph = doc.root.elements["section[@name='graph']"]

taskId = 0
graph.each_element("section[@name='node']") {|nodeElem|
  id = nodeElem.elements["attribute[@key='id']"][0].to_s
  label = nodeElem.elements["attribute[@key='label']"][0].to_s.gsub(/[\s\n*]/,"")

  # the label's format is task_name : instance-of; param = val, ...
  if not label =~ (/(\w+):(\w+)?(;(?:(\w+)=(\w+),)*(\w+)=(\w+))?/)
    puts "syntax error in \"#{label}\""
    exit(1)
  end

  tokens = label.match(/(\w+)(?::(\w+))?/)[1..-1]

  name = tokens[0] + taskId.to_s
  if tokens[1] == nil  # for convenience
    instance_of = tokens[0]
  else
    instance_of = tokens[1]
  end
  task = Task.new(name, instance_of)
  paramList = label.split(";")[1]
  paramList.scan(/(\w+)=(\w+)/) { |var, val| task.params[var] = val } if paramList != nil

  tasks[id] = task
  taskId += 1
}

graph.each_element("section[@name='edge']") {|edgeElem|
  source = tasks[edgeElem.elements["attribute[@key='source']"][0].to_s]
  target = tasks[edgeElem.elements["attribute[@key='target']"][0].to_s]

  if retrievals.has_key?(source)
    retrievals[source].add(target)
  else
    ns = Set.new
    ns.add(target)
    retrievals[source] = ns
  end

  if deps.has_key?(target)
    deps[target].add(source)
  else
    ns = Set.new
    ns.add(source)
    deps[target] = ns
  end
}

opId = 0      # to guarantee uniqueness
retrievals.each_pair { |src, adjList|
  adjList.each { |tgt|
    opName = "#{src.name}-trigger-#{tgt.name}-#{opId}"

    depString = ""
    deps[tgt].each {|d| depString.concat(" ^dep #{d.name}")}

    paramString = ""
    tgt.params.each_pair { |param_name, param_val|
      paramString.concat(" ^#{param_name} #{param_val}") 
    }

    # soar code for the actual operator
    op_proposal = <<EOF
sp {plan-memory*propose*#{opName}
   (state <s> ^name sorts
              ^planning <pl>)
   (<pl> ^execution-buffer <eb>
         ^retrieval-buffer <rb>
         ^newly-completed <nc> 
         ^completed <c>)
   (<nc> ^task.name #{src.name})
  -(<eb> ^task.name #{tgt.name})
  -(<rb> ^task.name #{tgt.name})
  -(<c> ^task.name #{tgt.name})
-->
   (<s> ^operator <o> +)
   (<o> ^name #{opName})}
EOF

    op_apply = <<EOF
sp {plan-memory*apply*#{opName}
   (state <s> ^name sorts
              ^operator <o>
              ^planning.retrieval-buffer <rb>)
   (<o> ^name #{opName})
-->
   (<rb> ^task <rt>)
   (<rt> ^name #{tgt.name}
         ^instance-of #{tgt.instance_of}
         ^params <p>
         #{depString})
   (<p> #{paramString})}
EOF

    puts op_proposal, op_apply
    opId += 1
  }
}
