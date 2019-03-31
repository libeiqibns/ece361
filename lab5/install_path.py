#!/usr/bin/python

import sys
import heapq
import re # For regex

import ryu_ofctl
from ryu_ofctl import *

def main(macHostA, macHostB):
    print "Installing flows for %s <==> %s" % (macHostA, macHostB)

    ##### FEEL FREE TO MODIFY ANYTHING HERE #####
    try:
        pathA2B = dijkstras(macHostA, macHostB)
        installPathFlows(macHostA, macHostB, pathA2B)
    except:
        raise


    return 0

# Optional helper function if you use suggested return format
def nodeDict(dpid, in_port, out_port):
    assert type(dpid) in (int, long)
    assert type(in_port) is int
    assert type(out_port) is int
    return {'dpid': dpid, 'in_port': in_port, 'out_port': out_port}

# Adds ingress port and egress port information to switches along path
def addPort2Path(source_port, dest_port, pathA2B):
    assert type(pathA2B[0]) is int
    assert type(source_port) is int
    assert type(dest_port) is int
    if pathA2B is None: 
        return None

    if len(pathA2B) == 1:
        pathA2B[0] = nodeDict(pathA2B[0], source_port, dest_port)
        return pathA2B

    in_port = source_port
    for i in range(len(pathA2B)-1):
        dpid = pathA2B[i]
        links = ryu_ofctl.listSwitchLinks(str(pathA2B[i]))
        links = links['links']
        for link in links:
            if (int(link['endpoint1']['dpid']) == pathA2B[i]) and (int(link['endpoint2']['dpid']) == pathA2B[i+1]):
                out_port = link['endpoint1']['port']
                pathA2B[i] = nodeDict(dpid, in_port, out_port)
                in_port = link['endpoint2']['port']
                break

    pathA2B[-1] = nodeDict(pathA2B[-1], in_port, dest_port)
    
    return pathA2B


# Installs end-to-end bi-directional flows in all switches
def installPathFlows(macHostA, macHostB, pathA2B):
    ##### YOUR CODE HERE #####
    for node in pathA2B:
        dpid = str(node['dpid'])
        in_port = node['in_port']
        out_port = node['out_port']

        flow = ryu_ofctl.FlowEntry()
        act = ryu_ofctl.OutputAction(out_port)
        flow.in_port = in_port
        flow.addAction(act)
        ryu_ofctl.insertFlow(dpid, flow)

        flow = ryu_ofctl.FlowEntry()
        act = ryu_ofctl.OutputAction(in_port)
        flow.in_port = out_port
        flow.addAction(act)
        ryu_ofctl.insertFlow(dpid, flow)
    return

# Returns List of neighbouring DPIDs
def findNeighbours(dpid):
    if type(dpid) not in (int, long) or dpid < 0:
        raise TypeError("DPID should be a positive integer value")

    neighbours = []

    ##### YOUR CODE HERE #####
    links = listSwitchLinks(str(dpid))
    links = links['links']

    for link in links:
        if int(link['endpoint1']['dpid']) == dpid:
            neighbours.append(int(link['endpoint2']['dpid']))

    return neighbours

# Calculates least distance path between A and B
# Returns detailed path (switch ID, input port, output port)
#   - Suggested data format is a List of Dictionaries
#       e.g.    [   {'dpid': 3, 'in_port': 1, 'out_port': 3},
#                   {'dpid': 2, 'in_port': 1, 'out_port': 2},
#                   {'dpid': 4, 'in_port': 3, 'out_port': 1},
#               ]
# Raises exception if either ingress or egress ports for the MACs can't be found
def dijkstras(macHostA, macHostB):

    # Optional variables and data structures
    INFINITY = float('inf')
    distanceFromA = {} # Key = node, value = distance
    leastDistNeighbour = {} # Key = node, value = neighbour node with least distance from A
    pathA2B = [] # Holds path information

    ##### YOUR CODE HERE #####
    source = ryu_ofctl.getMacIngressPort(macHostA)
    dest = ryu_ofctl.getMacIngressPort(macHostB)

    if source is None:
        raise ValueError("Cannot resolve MAC address %s" % macHostA)
    elif dest is None:
        raise ValueError("Cannot resolve MAC address %s" % macHostB)

    graph = {}
    graph['nodes'] = ryu_ofctl.listSwitches()
    graph['nodes'] = graph['nodes']['dpids']
    graph['nodes'] = map(int, graph['nodes'])
    graph['edges'] = ryu_ofctl.listLinks()
    graph['edges'] = graph['edges']['links']

    Q = []
    distanceFromA[int(source['dpid'])] = 0

    for node in graph['nodes']:
        if node != int(source['dpid']):
            distanceFromA[node] = INFINITY
        leastDistNeighbour[node] = None
        heapq.heappush(Q,(distanceFromA[node],node))

    while Q:
        min_node = heapq.heappop(Q)
        assert type(min_node[1]) is int
        if min_node[1] == int(dest['dpid']):
            break

        neighbours = findNeighbours(min_node[1])
        for neighbour in neighbours:
            alt_dist = min_node[0] + 1
            if alt_dist < distanceFromA[neighbour]:
                distanceFromA[neighbour] = alt_dist
                leastDistNeighbour[neighbour] = min_node[1]
                for q in Q:
                    if q[1] == neighbour:
                        Q.remove(q)
                        break
                heapq.heapify(Q)
                heapq.heappush(Q,(distanceFromA[neighbour],neighbour))

    cur_node = int(dest['dpid'])
    if (leastDistNeighbour[cur_node] is not None) or (cur_node == int(source['dpid'])):
        while cur_node is not None:
            pathA2B.insert(0, cur_node)
            cur_node = leastDistNeighbour[cur_node]

    pathA2B = addPort2Path(int(source['port']), int(dest['port']), pathA2B)
    # Some debugging output
    #print "leastDistNeighbour = %s" % leastDistNeighbour
    #print "distanceFromA = %s" % distanceFromA
    #print "pathAtoB = %s" % pathA2B

    return pathA2B



# Validates the MAC address format and returns a lowercase version of it
def validateMAC(mac):
    invalidMAC = re.findall('[^0-9a-f:]', mac.lower()) or len(mac) != 17
    if invalidMAC:
        raise ValueError("MAC address %s has an invalid format" % mac)

    return mac.lower()

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print "This script installs bi-directional flows between two hosts"
        print "Expected usage: install_path.py <hostA's MAC> <hostB's MAC>"
    else:
        macHostA = validateMAC(sys.argv[1])
        macHostB = validateMAC(sys.argv[2])

        sys.exit( main(macHostA, macHostB) )
