#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>
#include <stdio.h>
#include <getopt.h>
#include <limits>
#include <cmath>
#include <iomanip>

using namespace std;

option redirect[] = {
  {  "help",       no_argument, nullptr,  'h'},
  { "mode",  required_argument, nullptr,  'm'},
  { nullptr,                 0, nullptr, '\0'}
};

enum Location {
  SAFE = 1,
  BORDER = 2,
  DANGEROUS = 3,
};

struct part_a_coordinate {
  uint32_t prev;
  double distance;
  double x_coordinate;
  double y_coordinate;
  Location loc;
  bool visited;
};

struct part_b_c_coordinate {
  double x_coordinate;
  double y_coordinate;
};

class program {
public:
  vector<part_a_coordinate*> MST_coordinates;
  vector<part_b_c_coordinate*> TSP_coordinates;
  vector<uint32_t> MST_path;
  vector<size_t> best_tsp_output;
  double total_distance;
  vector<size_t> tsp_output;
  double current_best_distance;

  void delete_all_data() {
    delete_all_coordinates();
  }

  void print_MST() {
    cout  << setprecision(2) << fixed << total_distance << "\n";
    for(size_t i = 0; i < MST_path.size(); ++i) {
      cout << MST_path[i] << " ";
      ++i;
      cout << MST_path[i] << "\n";
    }
  }

  void print_FASTTSP() {
    cout  << setprecision(2) << fixed << total_distance << "\n";
    for(size_t i = 0; i < tsp_output.size() - 1; ++i) {
      cout << tsp_output[i] << " ";
    }
  }

  void print_OTPTSP_test(vector<size_t> &path) {
    for(size_t i = 0; i < path.size(); ++i) {
      cout << path[i] << " ";
    }
    cout << "\n";
  }
  
  void print_OPTTSP() {
    cout  << setprecision(2) << fixed << current_best_distance << "\n";
    for(size_t i = 0; i < best_tsp_output.size() - 1; ++i) {
      cout << best_tsp_output[i] << " ";
    }
    cout << best_tsp_output[best_tsp_output.size() - 1] << "\n";
  }

  void select_algorithm(string algorithm_type) {
    algorithm = algorithm_type; 
    if(algorithm_type == "MST") {
      MST_algorithm();
      print_MST();
    } else if((algorithm_type == "FASTTSP")) {
      fast_tsp_algorithm();
      print_FASTTSP();
    } else {
      optimal_tsp_algorithm();
      print_OPTTSP();
    }
  }

  Location find_location(int x, int y) {
    if(((x > 0) && (y > 0)) || ((x > 0) && (y < 0)) || ((x < 0) && (y > 0)) || ((x > 0) && (y == 0)) || ((x == 0) && (y > 0))) {
      return Location::SAFE;
    } else if ((x < 0) && (y < 0)) {
      return Location::DANGEROUS;
    }
    return Location::BORDER;
  }

private:
  string algorithm;
  uint32_t fixedLength;
  double estimate_distance;
  vector<size_t> current_tsp_output;

  void MST_algorithm() {
    total_distance = 0;
    uint32_t lowest_current_index;
    MST_coordinates.front()->distance = 0;
    
    for(size_t i = 0; i < MST_coordinates.size(); ++i) {
      double lowest_current_distance = std::numeric_limits<double>::infinity();
      for(size_t j = 0; j < MST_coordinates.size(); ++j) {
        if((!MST_coordinates[j]->visited) && (MST_coordinates[j]->distance < lowest_current_distance)) {
          lowest_current_distance = MST_coordinates[j]->distance;
          lowest_current_index = static_cast<uint32_t>(j);
        }
      }

      total_distance += sqrt(abs(lowest_current_distance));
      MST_coordinates[lowest_current_index]->visited = true;

      if(i != 0) {
        if(lowest_current_index < MST_coordinates[lowest_current_index]->prev) {
          MST_path.push_back(lowest_current_index);
          MST_path.push_back(MST_coordinates[lowest_current_index]->prev);
        } else {
          MST_path.push_back(MST_coordinates[lowest_current_index]->prev);
          MST_path.push_back(lowest_current_index);
        }
      }

      for(size_t j = 0; j < MST_coordinates.size(); ++j) {
        if(!MST_coordinates[j]->visited) {
          double test_distance = MST_calculate_distance(MST_coordinates[lowest_current_index], MST_coordinates[j]);
          if((test_distance < MST_coordinates[j]->distance) || (MST_coordinates[j]->distance == 0)) {
            MST_coordinates[j]->distance = test_distance;
            MST_coordinates[j]->prev = lowest_current_index;
          }
        }
      }
    }
  }

  double MST_calculate_distance(part_a_coordinate* c1, part_a_coordinate* c2) {
    double distance = 0.0;

    if((c1->loc == Location::SAFE) && (c2->loc == Location::DANGEROUS)) {
      return std::numeric_limits<double>::infinity();
    } else if((c2->loc == Location::SAFE) && (c1->loc == Location::DANGEROUS)) {
      return std::numeric_limits<double>::infinity();
    }

    distance = (c1->x_coordinate - c2->x_coordinate) * (c1->x_coordinate - c2->x_coordinate);
    distance = distance + ((c1->y_coordinate - c2->y_coordinate) * (c1->y_coordinate - c2->y_coordinate));

    return distance;
  }

  void fast_tsp_algorithm() {
    total_distance = 0.0;
    tsp_output.reserve(TSP_coordinates.size() + 1);
    tsp_output.push_back(0);
    tsp_output.push_back(0);

    double compare_begin_distance = TSP_calculate_distance(TSP_coordinates[1], TSP_coordinates[tsp_output[0]]);
    double compare_end_distance = TSP_calculate_distance(TSP_coordinates[1], TSP_coordinates[tsp_output[0]]);

    total_distance = total_distance + sqrt(abs(compare_begin_distance)) + sqrt(abs(compare_end_distance));
    tsp_output.insert(tsp_output.begin() + 1, 1);

    for(size_t i = 2; i < TSP_coordinates.size(); ++i) {
      double current_distance = std::numeric_limits<double>::infinity();
      size_t current_index = 0;

      for(size_t j = 0; j < tsp_output.size() - 1; ++j) {
        double compare_begin_distance = TSP_calculate_distance(TSP_coordinates[i], TSP_coordinates[tsp_output[j]]);
        double compare_end_distance = TSP_calculate_distance(TSP_coordinates[i], TSP_coordinates[tsp_output[j + 1]]);
        double initial_distance = TSP_calculate_distance(TSP_coordinates[tsp_output[j]], TSP_coordinates[tsp_output[j + 1]]);
        double diff_distance = (sqrt(compare_begin_distance) + sqrt(compare_end_distance)) - sqrt(initial_distance);

        if(diff_distance < current_distance) {
          current_distance = diff_distance;
          current_index = j;
        }
      }

      total_distance = total_distance + current_distance; 
      tsp_output.insert(tsp_output.begin() + static_cast<int>(current_index + 1), i);
    }
  }

  double TSP_calculate_distance(part_b_c_coordinate* c1, part_b_c_coordinate* c2) {
    double distance1 = (c1->x_coordinate - c2->x_coordinate) * (c1->x_coordinate - c2->x_coordinate);
    double distance2 = (c1->y_coordinate - c2->y_coordinate) * (c1->y_coordinate - c2->y_coordinate);

    return (distance1 + distance2);
  }

  void optimal_tsp_algorithm() {
    fast_tsp_algorithm();
    //total_distance = 336.75;
    //tsp_output = {0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8,  9, 10 };
    
    current_best_distance = total_distance;
    best_tsp_output = tsp_output;

    total_distance = 0;

    current_tsp_output = best_tsp_output;
    current_tsp_output.pop_back();
    fixedLength = 1;

    genPerms(current_tsp_output);
  }

  void genPerms(vector<size_t> &path) {
    if(fixedLength == path.size()) {
      double add_distance = sqrt(TSP_calculate_distance(TSP_coordinates[0], TSP_coordinates[path[fixedLength - 1]]));
      total_distance = total_distance + add_distance;
      if(total_distance < current_best_distance) {
        current_best_distance = total_distance;
        best_tsp_output = path;
      }

      total_distance = total_distance - add_distance;

      return;
    }

    if((best_tsp_output.size() - fixedLength) > 4) {
      if(!promising(path)) {
        return;
      }
    }

    for (size_t i = fixedLength; i < path.size(); ++i) {
      double add_distance = sqrt(TSP_calculate_distance(TSP_coordinates[path[fixedLength - 1]], TSP_coordinates[path[i]]));
      swap(path[fixedLength], path[i]);
      total_distance = total_distance + add_distance;
      fixedLength++;
      genPerms(path);
      total_distance = total_distance - add_distance;
      fixedLength--;
      swap(path[i], path[fixedLength]);
    }
  }

  bool promising(vector<size_t> &path) {
    estimate_distance = 0;
    estimate_distance = total_distance;

    double shortest_to_start = std::numeric_limits<double>::infinity();
    double shortest_to_end = std::numeric_limits<double>::infinity();

    for(size_t i = fixedLength; i < path.size(); ++i) {
      double to_start = TSP_calculate_distance(TSP_coordinates[0],TSP_coordinates[path[i]]);
      double to_end   = TSP_calculate_distance(TSP_coordinates[path[fixedLength - 1]],TSP_coordinates[path[i]]);

      if(to_start < shortest_to_start) {
        shortest_to_start = to_start;
      }

      if(to_end < shortest_to_end) {
        shortest_to_end = to_end;
      }
    }

    estimate_distance = estimate_distance + sqrt(shortest_to_end) + sqrt(shortest_to_start);

    if(estimate_distance > current_best_distance) {
      return false;
    }

    double MST_distance = MST_for_OPTTSP_algorithm();

    estimate_distance = estimate_distance + MST_distance;

    /*for (size_t i = 0; i < path.size(); ++i) {
      cout << setw(2) << path[i] << ' ';
    }
    cout << setw(4) << fixedLength << setw(10) << total_distance;
    cout << setw(10) << sqrt(shortest_to_start) << setw(10) << sqrt(shortest_to_end);
    cout << setw(10) << MST_distance << setw(10) << estimate_distance << "  ";*/

    if(estimate_distance < current_best_distance) {
      //cout << "true\n";
      return true;
    }
    //cout << "false\n";
    return false;
  }

  double MST_for_OPTTSP_algorithm() {
    double MST_total_distance = 0;
    vector<size_t> new_MST_vector;
    
    new_MST_vector = current_tsp_output;
    uint32_t lowest_current_index = 0;
    
    vector<double> distance_vector(new_MST_vector.size(), 0);
    vector<bool>    visited_vector(new_MST_vector.size(), false);
    
    for(size_t i = fixedLength; i < new_MST_vector.size(); ++i) {
      double lowest_current_distance = std::numeric_limits<double>::infinity();
      for(size_t j = fixedLength; j < new_MST_vector.size(); ++j) {        
        if((!visited_vector[j]) && (distance_vector[j] < lowest_current_distance)) {
          lowest_current_distance = distance_vector[j];
          lowest_current_index = static_cast<uint32_t>(j);
        }
      }
      MST_total_distance += sqrt(lowest_current_distance);
      visited_vector[lowest_current_index] = true;

      for(size_t j = fixedLength; j < distance_vector.size(); ++j) {
        if(!visited_vector[j]) {
          double test_distance = TSP_calculate_distance(TSP_coordinates[new_MST_vector[j]], TSP_coordinates[new_MST_vector[lowest_current_index]]);
          if((test_distance < distance_vector[j]) || (distance_vector[j] == 0)) {
            distance_vector[j] = test_distance;
          }
        }
      }
    }
    return MST_total_distance;
  }

  void delete_all_coordinates() {
    for(size_t i = 0; i < MST_coordinates.size(); ++i) {
      delete MST_coordinates[i];
    }
    MST_coordinates.clear();

    for(size_t i = 0; i < TSP_coordinates.size(); ++i) {
      delete TSP_coordinates[i];
    }
    TSP_coordinates.clear();
  }
};

int main(int argc, char* argv[]) {
  std::ios_base::sync_with_stdio(false);
  program zoo;
  cout << std::setprecision(2); //Always show 2 decimal places
  cout << std::fixed; //Disable scientific notation for large numbers
  int index = 0;
  int opt = 0;
  string algorithm_type;
  cout << fixed << showpoint << setprecision(2) << boolalpha;

  while ((opt = getopt_long(argc, argv, "hm:", redirect, &index)) != -1) {
    switch (opt) {
     case 'h': //work on the help line
        cout << "This program finds a path out of a puzzle if there is one\n";
        cout << "These are the valid command line arguments:\n";
        cout << "'-s' or '--statistics' will use a stack based path finding scheme\n";
        cout << "'-q' or '--queue' will use a queue based path finding scheme\n";
        cout << "'-o' or '--output' will indicate if you want a map or list output\n";
        cout << "'-h' or '--help' will print this message again and end the program\n";
        return 0;
        break;
      case 'm':
        algorithm_type = string(optarg);
        break;
      default:
        cerr << "Error: Unknown option\n";
        return 1;
    }
  }

  uint32_t num_coordinates;
  cin >> num_coordinates;

  int x_input;
  int y_input;
  bool safe = false;
  bool dangerous = false;
  bool wall = false;

  if(algorithm_type == "MST") {
    zoo.MST_coordinates.reserve(num_coordinates);

    while(cin >> x_input >> y_input) {
      part_a_coordinate* temp = new part_a_coordinate;
      temp->visited = false;
      temp->x_coordinate = x_input;
      temp->y_coordinate = y_input;
      temp->distance = std::numeric_limits<uint32_t>::infinity();
      temp->prev = std::numeric_limits<uint32_t>::infinity();
      temp->loc = zoo.find_location(x_input, y_input);

      zoo.MST_coordinates.push_back(temp);

      if(temp->loc == Location::BORDER) {
        wall = true;
      } else if(temp->loc == Location::DANGEROUS) {
        dangerous = true;
      } else {
        safe = true;
      }
    }
    if((safe) && (dangerous) && (!wall) && (algorithm_type == "MST")) {
      cerr << "Cannot construct MST\n";
      exit(1);
    }
  } else {
    zoo.TSP_coordinates.reserve(num_coordinates);

    while(cin >> x_input >> y_input) {
      part_b_c_coordinate* temp = new part_b_c_coordinate;
      temp->x_coordinate = x_input;
      temp->y_coordinate = y_input;

      zoo.TSP_coordinates.push_back(temp);
    }
  }

  zoo.select_algorithm(algorithm_type);

  zoo.delete_all_data();
  return 0;
};
