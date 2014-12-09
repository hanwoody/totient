#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <set>

#include "utils.cc"

using namespace std;
using namespace zmqpp;

#define PIECES_PATH  "./pieces/list"

// Begin peer state

string address, port, tracker_ip, tracker_port;
unordered_map<string, totient::entry> downloads;

// End peer state

void wake_up(socket &tracker) {
  set<string> pieces;
  ifstream i_file(PIECES_PATH);
  string line;
  while (getline(i_file, line)) {
    pieces.insert(line);
  }

  remove(PIECES_PATH);

  ofstream o_file(PIECES_PATH);

  message request;
  request << ADD << address << port << pieces.size();
  for (string piece : pieces) {
    o_file << piece << endl;
    request << piece;
  }

  tracker.send(request);

  i_file.close();
  o_file.close();
}

bool  share_file(socket &tracker, string &filename) {
  filename = "./files/" + filename;
  if (!file_exists(filename))
    return false;

  string command = "./totient_generator.sh " + filename + " " + tracker_ip + " " + tracker_port;
  cerr << string_color(command, BLUE) << endl;
  system(command.c_str());


  totient::entry totient_file(filename + ".totient");

  message request;

  request << ADD << address << port << totient_file.pieces.size();

  for (int i = 0; i < totient_file.pieces.size(); ++i)
    request << totient_file.pieces[i];

  tracker.send(request);

  return true;
}

bool download_file(socket &tracker, string &filename) {
  filename = "./totient/" + filename + ".totient";
  if (!file_exists(filename))
    return false;

  if (downloads.count(filename) == 0) {
    totient::entry entry(filename);
    downloads[filename] = entry;
  }

  return true;
}

void play_song(const string &filename) {

}

int main(int argc, char **argv) {

  if (argc < 5) {
    cout << "Usage: " << argv[0] << "address port tracker_ip tracker_port";
    exit(1);
  }

  address = argv[1];
  port    = argv[2];
  tracker_ip = argv[3];
  tracker_port = argv[4];
  const string tracker_endpoint = string("tcp://") + tracker_ip + ":" + tracker_port;

  context ctx;
  socket tracker(ctx, socket_type::dealer);
  tracker.connect(tracker_endpoint);

  wake_up(tracker);

  while (true) {
    string command;

    cout << "Totient P2P file sharing." << endl;
    cin >> command;

    if ((command == "q") or (command == "quit"))
      break;

    string filename;
    if (command == "share") {
      cout << "Enter the name of the file that you want to share (must be in the files dir)" << endl;
      cin >> filename;
      if (share_file(tracker, filename))
        cout << string_color("The file was successfully shared", GREEN) << endl;
      else
        cout << string_color("The file does not exist", RED) << endl;
    } else if (command == "download") {
      cout << "Enter the name of the file that you want to download (must be in the totient dir)" << endl;
      cin >> filename;
      if (download_file(tracker, filename))
        cout << string_color("Download in process", GREEN) << endl;
      else
        cout << string_color("The file does not exist", RED) << endl;
    } else if (command == "play") {
      cout << "Enter the name of the file that you want to hear (must bein the files dir)" << endl;
      cin >> filename;
      play_song(filename);
    }
  }

  cout << "Bye bye" << endl;

  return 0;
}
