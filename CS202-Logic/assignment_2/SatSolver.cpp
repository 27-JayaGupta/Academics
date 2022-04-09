#include <bits/stdc++.h>
#include <iostream>
#include <set>
#include <stdlib.h>
#include <chrono>
using namespace std;

// NO_VALUE means no value has been assigned to a particular literal

#define NO_VALUE -1

// TRUE means literal is set to true
#define TRUE 1

// FALSE sets literal to false
#define FALSE 0

// We use decision levels to measure the progress of the DPLL. Whenever we backtrack, we move one decision level down
// One decision level can contain multiple literals set to TRUE or FALSE
#define DECISION_MARK 0
 

 // Pointer is used to store pointer as well as index for a particular item in vector. Used in filtering clauses 
typedef struct Pointer {
  vector<int> *ptr;
  int index;
} Pointer;
 

// clauses stores the final list of clauses after filtering original

vector<vector<int>> clauses;

// posClauses store the Pointer of the clauses before filtering in which a particular literal appears as positive  
vector<vector<Pointer>> posClauses;

// negClauses store the Pointer of the clauses before filtering in which a particular literal appears as negative 
vector<vector<Pointer>> negClauses;

//permPosClauses store the Pointer of the clauses after filtering in which a particular literal appears as positive  
vector<vector<vector<int> *>> permPosClauses;

//permNegClauses store the Pointer of the clauses after filtering in which a particular literal appears as negative  
vector<vector<vector<int> *>> permNegClauses;

//stores value for all the literals 
vector<int> model;

// global variables
int numVariable, numClauses;
int nextChosenLiterals;
int indexOfNextLiteraltoPropagate;
int decision_level;
vector<int> modelStack;
vector<long long> positiveVariableConflicts;
vector<long long> negativeVariableConflicts;
clock_t start;
void print(vector<vector<int>> v) {
  for (int i = 0; i < v.size(); i++) {
    for (int j = 0; j < v[i].size(); j++) {
      cout << v[i][j] << " ";
    }
    cout << "\n";
  }
}

void printVector(vector<int> v) {

  for (int i = 0; i < v.size(); i++) {
    cout << v[i] << " ";
  }
  cout << endl;
}

// return value of the literal
int getModelValue(int literal) {
  if (literal > 0)
    return model[literal];
  else {
    if (model[-literal] == NO_VALUE)
      return NO_VALUE;
    else
      return 1 - model[-literal];
  }
}

// Input function which takes clauses in the DIMACS format
vector<vector<int>> takeInput(char* filename) {
  FILE *fp = fopen(filename, "r");
  char c;
  fscanf(fp, "%c", &c);
  while (c == 'c') {
    fscanf(fp, "%c", &c);
    while (c != '\n') {
      fscanf(fp, "%c", &c);
    }
    fscanf(fp, "%c", &c);
  }

  fscanf(fp, " cnf %d %d", &numVariable, &numClauses);
  // scanf("%d %d", &numVariable, &numClauses);

  vector<vector<int>> tempClauses;
  tempClauses.resize(numClauses + 1);
  posClauses.resize(numVariable + 1);
  negClauses.resize(numVariable + 1);
  positiveVariableConflicts.resize(numVariable + 1, 0);
  negativeVariableConflicts.resize(numVariable + 1, 0);
  model.resize(numVariable + 1, NO_VALUE);

  int literal;
  for (int i = 1; i <= numClauses; i++) {
    fscanf(fp, "%d", &literal);
    // scanf("%d", &literal);
    while (literal != 0) {
      tempClauses[i].push_back(literal);
      Pointer p;
      p.ptr = (vector<int> *)&clauses[i];
      p.index = i;
      if (literal > 0) {
        posClauses[literal].push_back(p);
      } else {
        negClauses[-literal].push_back(p);
      }
      fscanf(fp, "%d", &literal);
      // scanf("%d", &literal);
    }
  }

  // print(tempClauses);
  return tempClauses;
}

// return next decision literal = the one with highest activity(or most cases of conflicts)
int getNextDecisionLiteral() {

  long long maxActivity = 0;
  int maxActivityLiteral = 0;

  for (int i = numVariable; i >= 1; i--) {

    if (model[i] == NO_VALUE) {
      if (positiveVariableConflicts[i] >= maxActivity) {
        maxActivity = positiveVariableConflicts[i];
        maxActivityLiteral = i;
      }

      if (negativeVariableConflicts[i] >= maxActivity) {
        maxActivity = negativeVariableConflicts[i];
        maxActivityLiteral = -i;
      }
    }
  }

  return maxActivityLiteral;
}

// final output (tells to return the solver with sat or unsat)
void returnWithSatisfiability(bool result) {

  if (result) {
    cout << "SAT\n";
    cout << "Model: ";
    for (int i = 1; i <= numVariable; i++) {
      if (model[i] == FALSE)
        cout << -i << " ";
      else
        cout << i << " ";
    }
    
  } else {
    cout << "UNSAT\n";
  }
  cout << "\n-----------Execution Time: " << (double)(clock() - start)/CLOCKS_PER_SEC << " seconds---------------\n";
  exit(1);
}

void setValueofLiteral(int literal) {

  modelStack.push_back(literal);
  if (literal > 0) {
    model[literal] = TRUE;
  } else
    model[-literal] = FALSE;
}

void updateConflictDataClause(vector<int> claus) {

  for (int i = 0; i < claus.size(); i++) {
    if (claus[i] > 0) {
      positiveVariableConflicts[claus[i]] ++;
    } else {
      negativeVariableConflicts[claus[i]] ++;
    }
  }
}

// checks if giving the literal a intepretation causes a conflict or not
//(conflict means does any clause becomes false)
bool givesConflict() {
  // cout << "here in Conflict\n";

  while (indexOfNextLiteraltoPropagate < modelStack.size()) {

    // cout << "inside while loop\n";
    int literal = modelStack[indexOfNextLiteraltoPropagate];
    ++indexOfNextLiteraltoPropagate;

    int isUndefined = 0;
    int lastUndefinedLiteral;
    bool isTrue = 0;

    vector<vector<int> *> clausesToPropagate =
        literal > 0 ? permNegClauses[literal] : permPosClauses[-literal];

    // cout << "size: " << clausesToPropagate.size() << "\n";
    for (int i = 0; i < clausesToPropagate.size(); i++) {
      isUndefined = 0;
      isTrue = 0;
      lastUndefinedLiteral = 0;
      vector<int> claus = (vector<int>)(*(clausesToPropagate[i]));
      // cout << "conflict vector traversal: ";
      // printVector(claus);

      for (int j = 0; j < claus.size(); j++) {
        int val = getModelValue(claus[j]);
        if (val == TRUE) {
          isTrue = 1;
        }

        else if (val == NO_VALUE) {
          isUndefined++;
          lastUndefinedLiteral = claus[j];
        }
      }

      if (isTrue == 0 && isUndefined == 0) {
        updateConflictDataClause(claus);
        return true;
      }
      if (isTrue == 0 && isUndefined == 1) {
        setValueofLiteral(lastUndefinedLiteral);
      }
    }
  }

  return false;
}

//basic DPLL backtrack algorithm if a given interpretation makes any of the clause unsat 
//its value is reverted and then pushed in the stack
void backtrack() {
  int i = modelStack.size() - 1;
  int literal;

  while (modelStack[i] != DECISION_MARK) {
    literal = modelStack[i];
    model[abs(literal)] = NO_VALUE;
    --i;
    modelStack.pop_back();
  }

  modelStack.pop_back();
  --decision_level;
  indexOfNextLiteraltoPropagate = modelStack.size();
  setValueofLiteral(-literal);
}

// main function that executes the entire algorithm
void solveDPLL() {

  indexOfNextLiteraltoPropagate = 0;
  decision_level = 0;

  while (1) {

    while (givesConflict()) {
      if (decision_level == 0) {
        returnWithSatisfiability(FALSE);
      }
      backtrack();
    }

    int nextDecisionLiteral = getNextDecisionLiteral();
    // cout << "Decision literal: " << nextDecisionLiteral << endl;

    if (nextDecisionLiteral == 0) {
      returnWithSatisfiability(TRUE);
    }

    modelStack.push_back(DECISION_MARK);
    ++indexOfNextLiteraltoPropagate;
    ++decision_level;

    setValueofLiteral(nextDecisionLiteral);
    // printVector(modelStack);
  }
}

//checks if a clause is a unit clause of not (clause of length 1 ) 
//and makes it sat (by giving it the required interpretation)
bool isUnitClause(vector<int> &claus) {

  int literal = claus[0];

  if (literal > 0) {
    if (model[literal] == FALSE) {
      cout << "UNSAT\n";
      return false;
    } else {
      model[literal] = TRUE;
      return true;
    }
  } else {
    if (model[-literal] == TRUE) {
      cout << "UNSAT\n";
      return false;
    } else {
      model[-literal] = FALSE;
      return true;
    }
  }
}

int value(int literal) {
  if (literal > 0)
    return numVariable + literal;
  else
    return -literal;
}

// checks if a clause is a tautology(contains a and - a both in the clause)
bool isTautology(vector<int> &claus) {
  vector<int> isPresent(2 * numVariable + 1, 0);
  for (int i = 0; i < claus.size(); i++) {
    if (isPresent[value(-claus[i])]) {
      return true;
    } else {
      isPresent[value(claus[i])] = 1;
    }
  }
  return false;
}

//for a given literal it checks if a literal just appears in the form of a(or -a) in all the clasues and 
//return the index of all such clauses
set<int> isPureLiteral(vector<vector<int>> &tempClauses) {
  set<int> pureLiteralIndex;
  for (int i = 1; i <= numVariable; i++) {
    if (posClauses[i].size() != 0 && negClauses[i].size() == 0) {
      model[i] = TRUE;
      for (int j = 0; j < posClauses[i].size(); j++) {
        pureLiteralIndex.insert(posClauses[i][j].index);
      }
    } else if (posClauses[i].size() == 0 && negClauses[i].size() != 0) {
      model[i] = FALSE;
      for (int j = 0; j < negClauses[i].size(); j++) {
        pureLiteralIndex.insert(negClauses[i][j].index);
      }
    }
  }

  return pureLiteralIndex;
}

//main function which filter out the naive clasues (like Unit Clause , Tautologies and Pure Literal Clauses)
void filterClauses(vector<vector<int>> &tempClauses) {
  int tempNumClauses = numClauses;

  set<int> pureLiteralIndex = isPureLiteral(tempClauses);

  for (int i = 1; i <= tempNumClauses; i++) {

    if (i == *(pureLiteralIndex.begin())) {
      pureLiteralIndex.erase(pureLiteralIndex.begin());
      continue;
    }
    if (tempClauses[i].size() == 1) {

      bool isSatisfiableUnit = isUnitClause(tempClauses[i]);
      if (isSatisfiableUnit == false) {
        exit(-1);
      } else {

        numClauses--;
        continue;
      }
    }

    bool isTautologyClause = isTautology(tempClauses[i]);
    if (isTautologyClause) {
      numClauses--;
      continue;
    }

    clauses.push_back(tempClauses[i]);
  }
}


// a helper function
// after the final clause list is finalised (after filtering) , permanent Positive and Negative clause pointer list corresponding to literals
//is created in which the pointer to clauses where the literal appears as a(Pos) or -a(Neg) is stored
void initializePosandNegClauses() {

  permNegClauses.resize(numVariable + 1);
  permPosClauses.resize(numVariable + 1);

  for (int i = 0; i < clauses.size(); i++) {
    for (int j = 0; j < clauses[i].size(); j++) {
      int literal = clauses[i][j];
      if (getModelValue(literal) == FALSE)
        continue;

      if (literal > 0) {

        permPosClauses[literal].push_back((vector<int> *)&clauses[i]);
      } else {
        permNegClauses[-literal].push_back((vector<int> *)&clauses[i]);
      }
    }
  }
}

int main(int argc, char* argv[]) {

  start = clock();  
  vector<vector<int>> tempClauses = takeInput(argv[1]);

  filterClauses(tempClauses);

  // cout << clauses.size() << endl;

  initializePosandNegClauses();
  // cout << permPosClauses[1].size() << endl;
  // cout << permNegClauses[1].size() << endl;

  solveDPLL();

}
