#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdlib> // Necesario para rand()
#include <ctime>   // Necesario para inicializar la semilla de rand()
#include <iomanip>
#include <iostream>
#include <math.h>
#include <random>
#include <set>
#include <vector>

using namespace sf;
using namespace std;

int aleatorio() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, 10000);
  return dist(gen);
}

int toint(vector<int> v) {
  int num = 0;
  for (int i = v.size() - 1, pot = 0; i >= 0; i--, pot++) {
    if (v[i] == 1) {
      num +=
          (1 << pot); // Utilizar desplazamiento de bits en lugar de pow(2, pot)
    }
  }
  return num;
}

vector<int> toint2(vector<int> v, int var = 1) {
  vector<int> vec(var, 0);
  int genesPorVariable = v.size() / var; // Número de genes por variable

  for (int i = 0; i < var; i++) {
    int valor = 0;

    // Calcula el valor entero para la variable actual
    for (int j = i * genesPorVariable; j < (i + 1) * genesPorVariable; j++) {
      valor = (valor << 1) | v[j];
    }

    vec[i] = valor;
  }

  return vec;
}

double pi = 3.1415;
double funcion(int x) { return x * x; }
double funcion2(int x, int y) {
  return 20 + pow(x, 2) + pow(y, 2) - 10 * cos(2 * pi * x) + cos(2 * pi * y);
}
double funcion3(int x, int y, int z) {
  if (z == 0)
    return 0;
  return (x + y) + z;
}

struct individuo {
  vector<int> genes;
  int variables = 0;
  int valor = 0;
  vector<int> valores;
  float funval = 0;
  double psle = 0;
  double expval = 0;
  float currval = 0;
  individuo(int i, int v = 1) {
    genes.resize(i, 0);
    variables = v;
    valores.resize(variables);
  }
  individuo(vector<int> v) { genes = v; }
  void random() {
    for (int &gen : genes) {
      gen = rand() % 2;
    }
  }
  void print() {
    for (int gen : genes) {
      cout << gen;
    }
  }
  void setvalor() {
    if (variables == 1) {
      valor = toint(genes);
      valores[0] = valor;
    } else {
      valores = toint2(genes, variables);
    }
  }
  void setfunval() {
    if (variables == 1) {
      funval = funcion(valor);
    } else if (variables == 2) {
      funval = funcion2(valores[0], valores[1]);
    } else {
      funval = funcion3(valores[0], valores[1], valores[2]);
    }
  }
};

struct Algoritmo {
  vector<pair<int, int>> grafico;
  vector<individuo> poblacion;
  int suma = 0;
  int media = 0;
  int var;
  int maxv = -10000;
  Algoritmo(int pob, int gen, int _var = 1) {
    srand(static_cast<unsigned int>(time(nullptr)));
    poblacion.resize(pob, individuo(gen));
    for (auto &it : poblacion) {
      it.random();
      it.variables = _var;
    }
    var = _var;
  }
  void print() {
    for (auto &it : poblacion) {
      it.print();
      cout << endl;
    }
  }
  void printall() {
    // for (auto& it : poblacion) {
    //     it.print();
    //     cout << "\t|\t" << it.valor << "\t|\t" << it.funval << "\t|\t" <<
    //     it.psle << "\t|\t" << it.expval << "\t|\t" << it.currval; cout <<
    //     endl;
    // }
    for (auto &it : poblacion) {
      it.print();
      // cout << ";i;";
      for (int i = 0; i < it.valores.size(); i++) {
        cout << "\t|\t" << it.valores[i];
      }
      // cout << ";i;";
      cout << "\t|\t" << it.funval << "\t|\t" << it.psle << "\t|\t" << it.expval
           << "\t|\t" << it.currval;
      cout << endl;
    }
    cout << "\t\t\t\t" << suma << endl;
    cout << "\t\t\t\t" << media << endl;
    cout << "\t\t\t\t" << maxv;
  }
  void actualizar_currval() {
    // Calcula la suma total de expval en la población actual
    double suma_expval = 0.0;
    for (const individuo &ind : poblacion) {
      suma_expval += ind.expval;
    }

    // Calcula el número total de descendientes que se deben asignar
    int total_descendientes = poblacion.size();

    // Calcula los valores iniciales de currval basados en expval
    for (individuo &ind : poblacion) {
      ind.currval =
          static_cast<int>((ind.expval / suma_expval) * total_descendientes);
    }

    // Calcula la diferencia entre la suma actual de currval y el tamaño de la
    // población
    int diferencia = total_descendientes - calcular_suma_currval();

    // Ajusta los valores de currval para que la suma sea igual al tamaño de la
    // población
    for (int i = 0; i < diferencia; i++) {
      poblacion[i].currval++;
    }
  }

  int calcular_suma_currval() {
    int suma = 0;
    for (const individuo &ind : poblacion) {
      suma += ind.currval;
    }
    return suma;
  }

  void mutar(individuo &ind, double probabilidad) {
    for (int i = 0; i < ind.genes.size(); i++) {
      double random_prob = static_cast<double>(rand()) / RAND_MAX;
      if (random_prob < probabilidad) {
        // Cambia un bit aleatorio
        ind.genes[i] = 1 - ind.genes[i];
      }
    }
  }

  void setall() {
    for (auto &it : poblacion) {
      it.setvalor();
      it.setfunval();
      suma += it.funval;
      if (it.funval > maxv)
        maxv = it.funval;
    }
    media = suma / poblacion.size();
    for (auto &it : poblacion) {
      it.psle = static_cast<double>(it.funval) / suma;
      if (media != 0) {
        it.expval = static_cast<double>(it.funval) / media;
      } else
        it.expval = 0;
      // it.currval = round(it.expval);
    }
    actualizar_currval();
  }
  void cruzar_poblacion() {
    // Crea un vector para almacenar la descendencia
    vector<individuo> descendencia;

    // Crea un conjunto para realizar un seguimiento de los índices de
    // individuos utilizados
    set<int> individuos_utilizados;

    // Realiza el cruce en pares de individuos
    for (size_t i = 0; i < poblacion.size(); i += 2) {
      // Elige dos padres aleatorios que no se hayan utilizado previamente
      int indice_padre1, indice_padre2;
      do {
        indice_padre1 = aleatorio() % poblacion.size();
      } while (individuos_utilizados.count(indice_padre1) > 0);

      do {
        indice_padre2 = aleatorio() % poblacion.size();
      } while (individuos_utilizados.count(indice_padre2) > 0 ||
               indice_padre1 == indice_padre2);

      // Agrega los índices de los padres utilizados al conjunto
      individuos_utilizados.insert(indice_padre1);
      individuos_utilizados.insert(indice_padre2);

      // Punto de cruce aleatorio (varía para cada par de padres)
      int punto_de_cruce1 = aleatorio() % poblacion[i].genes.size();
      int punto_de_cruce2 = aleatorio() % poblacion[i].genes.size();

      // Crea a los hijos
      individuo hijo1(poblacion[i].genes.size(), var);
      individuo hijo2(poblacion[i].genes.size(), var);

      // Realiza el cruce en los puntos de cruce aleatorios
      for (int j = 0; j < punto_de_cruce1; j++) {
        hijo1.genes[j] = poblacion[indice_padre1].genes[j];
        hijo2.genes[j] = poblacion[indice_padre2].genes[j];
      }

      for (int j = punto_de_cruce1; j < poblacion[i].genes.size(); j++) {
        hijo1.genes[j] = poblacion[indice_padre2].genes[j];
        hijo2.genes[j] = poblacion[indice_padre1].genes[j];
      }

      // Agrega los hijos a la descendencia
      descendencia.push_back(hijo1);
      descendencia.push_back(hijo2);

      ////// Imprime el cruce
      // cout << "Cruce entre individuo " << indice_padre1 << " y individuo " <<
      // indice_padre2 << ":" << endl; cout << "Punto de cruce 1: " <<
      // punto_de_cruce1 << endl; cout << "Padre 1: "; for (int j :
      // poblacion[indice_padre1].genes) {
      //     cout << j;
      // }
      // cout << endl;
      // cout << "Padre 2: ";
      // for (int j : poblacion[indice_padre2].genes) {
      //     cout << j;
      // }
      // cout << endl;
      // cout << "Hijo 1: ";
      // for (int j : hijo1.genes) {
      //     cout << j;
      // }
      // cout << endl;
      // cout << "Hijo 2: ";
      // for (int j : hijo2.genes) {
      //     cout << j;
      // }
      // cout << endl << endl;
    }

    // Reemplaza la población actual con la descendencia
    poblacion = descendencia;
  }

  void remplazar() {
    vector<individuo> aux;
    for (auto &it : poblacion) {
      for (int i = 0; i < it.currval; i++) {
        aux.push_back(it);
      }
    }
    for (auto &it : aux) {
      mutar(it, 0.005); // Probabilidad de mutación del 5%
    }
    poblacion = aux;

    cruzar_poblacion();
    setall();
  }
  void make(int eras) {
    for (int i = 0; i < eras; i++) {
      setall();
      printall();
      grafico.push_back(make_pair(i, media));
      remplazar();
      suma = 0;
      maxv = 0;
      cout << endl << endl;
    }
  }
};

RenderWindow window(VideoMode(700, 700), "SFML Circle Example");
Event event;

int main() {
  int poblacion = 4;
  int bits = 5;
  int ejemplares = 1;
  int gen = 100;

  individuo aux(bits, ejemplares);
  aux.genes = vector<int>(bits, 1);
  aux.setvalor();
  aux.setfunval();
  Algoritmo a(poblacion, bits, ejemplares);
  a.make(gen);
  float tope = aux.funval;
  // float tope = a.grafico[gen - 1].second;
  // cout << "TOPE ES=" << tope;
  float div = 500 / tope;
  window.setFramerateLimit(30);
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear();
    VertexArray ejeX(Lines, 2);
    ejeX[0].position = Vector2f(100, 100);
    ejeX[1].position = Vector2f(100, 600);
    window.draw(ejeX);

    VertexArray ejeY(Lines, 2);
    ejeY[0].position = Vector2f(600, 600);
    ejeY[1].position = Vector2f(100, 600);
    window.draw(ejeY);
    float divi = 500 / gen;
    for (int i = 0; i < gen - 1; i += true) {
      VertexArray line(Lines, 2);
      // cout << 600 - a.grafico[i].second * div << endl;
      line[1].position =
          Vector2f(100 + (i * divi), 600 - a.grafico[i].second * div);
      line[0].position =
          Vector2f(100 + ((i + 1) * divi), 600 - a.grafico[i + 1].second * div);
      window.draw(line);
    }

    window.display();
  }
}
