#!/usr/bin/env ruby

require 'json'
require 'fileutils'

def read_raw_data(original_file_name)
  num_ports               = 0
  distances               = Array.new
  posx                    = Array.new
  posy                    = Array.new
  demand                  = Array.new
  draught                 = Array.new
  parsing_distances       = false
  parsing_distances_line  = 0

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
  
  return {
    :original_file_name => original_file_name,
    :num_ports          => num_ports,
    :distances          => distances
  }
end

def generate_instances(input_data)
  instances     = Array.new
  max_demand    = 99
  min_demand    = 1
  avg_demand    = ((min_demand + max_demand).to_f / 2).to_i
  instance_name = File.basename(input_data[:original_file_name], ".tsp").split("_").first
  depot         = (0..(input_data[:num_ports]-1)).to_a.sample
  normal_ports  = (0..(input_data[:num_ports]-1)).to_a - [depot]

  #[10, 14, 18, 22].each do |n|
  [5, 8, 12, 16, 20].each do |n|
    requests = Array.new
        
    0.upto(n-1) do |request|
      origin            = normal_ports.sample
      destination       = (normal_ports - [origin]).sample
      demand            = min_demand + Random.rand(max_demand - min_demand + 1)
      requests[request] = {
        :origin       => origin,
        :destination  => destination,
        :demand       => demand
      }
    end

    [0.1, 0.3, 0.5, 2].each do |h|
      q                 = ([h * n * avg_demand, requests.map{|r| r[:demand]}.max].max).to_i
      possible_k_values = (h == 2 ? [1.0] : [0.0, 0.33, 0.67, 1.0])
      
      possible_k_values.each do |k|
        ports   = Array.new
        draught = Array.new
        
        0.upto(input_data[:num_ports]-1) do |port|
          rnd = Random.rand
  
          if rnd <= k
            draught[port] = q
          else
            max_port_demand = 0
            0.upto(n-1) do |request|
              if (port == requests[request][:origin] || port == requests[request][:destination]) && requests[request][:demand] > max_port_demand
                max_port_demand = requests[request][:demand]
              end
            end
    
            draught[port] = (max_port_demand + Random.rand(q - max_port_demand + 1)).to_i
          end
          
          draught[port] = (2 * n * max_demand).to_i if port == depot
  
          ports[port] = {
            :id       => port,
            :draught  => draught[port],
            :depot    => (port == depot)
          }
        end

        instances.push({
          :num_ports      => input_data[:num_ports],
          :n              => n,
          :h              => h,
          :k              => k,
          :q              => q,
          :ports          => ports,
          :requests       => requests,
          :capacity       => q,
          :distances      => input_data[:distances],
          :instance_name  => instance_name
        })
      end
    end
  end
  
  return instances
end

def read_process_and_print(original_file_name)
  input_data  = read_raw_data(original_file_name)
  instances   = generate_instances(input_data)
  
  instances.each do |instance|
    new_file_name = "../data/new/"
    # if instance[:h] == 2
    #   new_file_name += "tsppd/"
    # else
    #   if instance[:k] == 1.0
    #     new_file_name += "ctsppd/"
    #   else
    #     new_file_name += "tsppddl/"
    #   end
    # end
    new_file_name += "#{instance[:instance_name]}_#{instance[:n]}_#{instance[:h]}_#{instance[:k]}.json"
    
    dirname = File.dirname(new_file_name)
    unless File.directory?(dirname)
      FileUtils.mkdir_p(dirname)
    end
    
    data = {
      :num_ports => instance[:num_ports],
      :ports => instance[:ports],
      :num_requests => instance[:n],
      :requests => instance[:requests],
      :capacity => instance[:q],
      :distances => instance[:distances]
    }
    
    File.open(new_file_name, "w") {|file| file.write JSON.pretty_generate(data)}
  end
end

read_process_and_print(ARGV[0])