#!/Users/alberto/.rvm/rubies/ruby-2.1.5/bin/ruby

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
      distances[parsing_distances_line] = line.split.map(&:to_i).map{|d| d == 1 ? 0 : d }
    end
    parsing_distances_line += 1
  end
end

ports = Array.new
requests = Array.new
origins = Array.new
destinations = Array.new
demand = Array.new
draught = Array.new

max_demand = 99
min_demand = 1
avg_demand = ((min_demand + max_demand) / 2).to_i

n = ARGV[1].to_i
h = ARGV[2].to_i
k = ARGV[3].to_f

instance_name = File.basename(ARGV[0], ".dat").split("_").first

depot = (1..num_ports).to_a.sample
normal_ports = ports - [depot]

0.upto(n-1) do |request|
  origins[request] = normal_ports.sample
  destinations[request] = (normal_ports - [origins[i]]).sample
  demand[request] = min_demand + Random.rand(max_demand - min_demand + 1)
  requests[request] = {
    :origin => origins[request],
    :destination => destinations[request],
    :demand => demand[request]
  }
end

q = [h * avg_demand, demand.max].max

0.upto(num_ports-1) do |port|
  rnd = Random.rand
  
  if rnd <= k
    draught[port] = q
  else
    max_port_demand = 0
    0.upto(n-1) do |request|
      if (port == origins[request] || port == destinations[request]) && demand[request] > max_port_demand
        max_port_demand = demand[request]
      end
    end
    
    draught[port] = max_port_demand + Random.rand(q - max_port_demand)
  end
  
  ports[i] = {
    :id => port,
    :draught => draught[port],
    :depot => (port == depot)
  }
end

data = {
  :num_ports => num_ports,
  :ports => ports,
  :requests => requests,
  :capacity => q,
  :distances => distances
}

new_file_name = "../data/new/#{instance_name}_#{n}_#{h}_#{k}.json"

File.open(new_file_name, "w") {|file| file.write JSON.pretty_generate(data)}