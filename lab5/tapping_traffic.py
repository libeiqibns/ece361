import ryu_ofctl

# part 1.3 tapping traffic
# h2 tapping traffic between h1 and h3

print "Tapping traffic for h1 <==> h3"

# get flow, set parameter
flow = ryu_ofctl.FlowEntry()
dpid = str(1)

# add action
act1 = ryu_ofctl.OutputAction(1)
act2 = ryu_ofctl.OutputAction(2)
act3 = ryu_ofctl.OutputAction(3)

# flow 1, from h1 to h2 & h3
flow.in_port = 1
flow.addAction(act2)
flow.addAction(act3)
ryu_ofctl.insertFlow(dpid, flow)

# flow 2, from h3 to h1 & h2
flow.in_port = 3
flow.addAction(act1)
flow.addAction(act2)
ryu_ofctl.insertFlow(dpid, flow)

print "Done tapping all traffic h1 <==> h3, with host h2 monitoring"
