#!/usr/bin/env ruby

require 'json'

original_file_name = ARGV[0]
num_ports = 0
distances = Array.new
posx = Array.new
posy = Array.new
demand = Array.new
draught = Array.new
data = Hash.new

parsing_distances = false
parsing_distances_line = 0

File.open(original_file_name, "r").each do |line|
  next if line[0] == "!"
  
  unless parsing_distances
    if line =~ /^N\:\s*\d+/
      num_ports = line[/\d+/].to_i
    end
    if line =~ /^Distance\:/
      parsing_distances = true
    end
    if line =~ /^PosX\:/
      label, parenthesis, *posx = line.split
      posx.map!(&:to_i)
    end
    if line =~ /^PosY\:/
      label, parenthesis, *posy = line.split
      posy.map!(&:to_i)
    end
    if line =~ /^Demand\:/
      label, parenthesis, *demand = line.split
      demand.map!(&:to_i)
    end
    if line =~ /^Draft\:/
      label, parenthesis, *draught = line.split
      draught.map!(&:to_i)
    end
  else
    if parsing_distances_line >= num_ports
      parsing_distances = false
    else
      distances[parsing_distances_line] = line.split.map(&:to_i)
    end
    parsing_distances_line += 1
  end
end

ports = Array.new
requests = Array.new
origins = Array.new
capacity = 0

0.upto(num_ports - 1) do |i|
  ports[i] = {:id => i, :draught => draught[i], :depot => (i == 0 ? true : false), :x => posx[i], :y => posy[i]}

  origins[i] = ((1..num_ports-1).to_a - [i]).sample
  demand[i] = [draught[origins[i]], draught[i], ((num_ports/8) + ((-1) ** Random.rand(2)) * Random.rand(0..(num_ports/16))).round].min
  capacity = (num_ports + ((-1) ** Random.rand(2)) * Random.rand(0..(num_ports/4))).round
    
  unless i == 0
    requests[i-1] = {:origin => origins[i], :destination => i, :demand => demand[i]}
  end
end

data = {
  :num_ports => num_ports,
  :ports => ports,
  :num_requests => num_ports - 1,
  :requests => requests,
  :capacity => capacity,
  :distances => distances
}

new_file_name = "../data/" + File.basename(ARGV[0], ".dat") + ".json"

File.open(new_file_name, "w") {|file| file.write JSON.pretty_generate(data) }