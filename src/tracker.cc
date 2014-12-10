#include <bits/stdc++.h>
#include <zmqpp/zmqpp.hpp>
#include "utils.cc"

using namespace std;
using namespace zmqpp;

unordered_map<string, set<pair<string, string>>> parts;

void add_rem_peer(message &request, bool add = true) {
  string ip, port, sha;
  size_t sha_number;

  request >> ip >> port >> sha_number;

  cout << "parts : " << sha_number << endl;
  while (sha_number-- > 0) {
    request >> sha;
    if (add)
      parts[sha].insert({ip, port});
    else
      parts[sha].erase({ip, port});
    cout << string_color("Added part " + sha) << endl;
  }

}

void search_part(message &request) {
  string sha, ip, port;

  request >> sha >> ip >> port;

  context ctx;
  string temp_peer_endpoint = "tcp://" + ip + ":" + port;
  socket temp_peer(ctx, socket_type::dealer);
  temp_peer.connect(temp_peer_endpoint);

  message response;

  if (parts[sha].size() > 0) {
    response << parts[sha].size();
    for (auto it : parts[sha])
      response << it.first << it.second;
  } else {
    response << "NF";
  }

  temp_peer.send(response);
}

void dispatch_peer(message &request) {
  string id, command;
  request >> id >> command;

  if (command == "add") {
    add_rem_peer(request);
  } else if (command == "rem") { //Losing my religion
    add_rem_peer (request, false);
  } else if (command == "search"){
    search_part (request);
  }
}


int main(int argc, char ** argv) {

  if (argc < 2) {
    cout << "Usage : " << argv[0] << " port" << endl;
    exit(1);
  }

  const string port = argv[1];
  const string peer_endpoint = "tcp://*:" + port;
  cout << string_color("Running tracker at port " + port) << endl;

  context ctx;

  socket peers(ctx, socket_type::router);
  peers.bind(peer_endpoint);

  poller pol;
  pol.add(peers);

  message request;

  while (true){
    if (pol.poll()){
      if (pol.has_input(peers)){
        peers.receive(request);
        dispatch_peer(request);
      }
    }
  }

  return 0;
}


