#!/Users/alberto/.rvm/rubies/ruby-2.1.2/bin/ruby

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

# Forget about demands and draught in the original instances
demand = Array.new
draught = Array.new

max_load = 90
min_load = 10
avg_load = ((min_load + max_load) / 2).to_i

h = ARGV[1].to_f
k = ARGV[2].to_f
instance_name = File.basename(ARGV[0], ".dat").split("_").first

q = (num_ports * h * avg_load).to_i

0.upto(num_ports - 1) do |i|
  draught[i] = ((Random.rand >= k) ? q : (q/2 + Random.rand(q/2 + 2)))
  origins[i] = ((1..num_ports-1).to_a - [i]).sample
end

0.upto(num_ports - 1) do |i|
  x = Math.log(min_load.to_f / max_load.to_f)
  demand[i] = [draught[origins[i]], draught[i], min_load * Math.exp(Random.rand(-1.1 * x))].min.to_i
  # demand[i] = [draught[origins[i]], draught[i], (min_load + Random.rand(max_load - min_load + 1))].min.to_i
  ports[i] = {:id => i, :draught => draught[i], :depot => (i == 0 ? true : false), :x => posx[i], :y => posy[i]}
    
  unless i == 0
    requests[i-1] = {:origin => origins[i], :destination => i, :demand => demand[i]}
  end
end

data = {
  :num_ports => num_ports,
  :ports => ports,
  :num_requests => num_ports - 1,
  :requests => requests,
  :capacity => q,
  :distances => distances
}

new_file_name = "../data/new/#{instance_name}_#{h}_#{k}.json"

File.open(new_file_name, "w") {|file| file.write JSON.pretty_generate(data)}