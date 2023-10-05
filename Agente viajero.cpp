#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

const int NUM_CIUDADES = 10;

struct Ciudad {
    int x, y;
};

double distancia(const Ciudad& ciudad1, const Ciudad& ciudad2) {
    int dx = ciudad1.x - ciudad2.x;
    int dy = ciudad1.y - ciudad2.y;
    return sqrt(dx * dx + dy * dy);
}

struct Individuo {
    vector<int> recorrido;
    double fitness;

    Individuo(const vector<Ciudad>& ciudades) {
        recorrido.resize(NUM_CIUDADES);
        for (int i = 0; i < NUM_CIUDADES; ++i) {
            recorrido[i] = i;
        }
        random_shuffle(recorrido.begin() + 1, recorrido.end());
        fitness = calcularFitness(ciudades);
    }

    double calcularFitness(const vector<Ciudad>& ciudades) {
        double dist = 0.0;
        for (int i = 0; i < NUM_CIUDADES - 1; ++i) {
            dist += distancia(ciudades[recorrido[i]], ciudades[recorrido[i + 1]]);
        }
        dist += distancia(ciudades[recorrido[NUM_CIUDADES - 1]], ciudades[recorrido[0]]);
        return 1.0 / dist;
    }
};

Individuo cruzar(const Individuo& padre1, const Individuo& padre2, const vector<Ciudad>& ciudades) {
    int punto1 = rand() % NUM_CIUDADES;
    int punto2 = rand() % NUM_CIUDADES;
    if (punto1 > punto2) {
        swap(punto1, punto2);
    }

    vector<int> hijo(NUM_CIUDADES, -1);

    for (int i = punto1; i <= punto2; ++i) {
        hijo[i] = padre1.recorrido[i];
    }

    int j = 0;
    for (int i = 0; i < NUM_CIUDADES; ++i) {
        if (j == punto1) {
            j = punto2 + 1;
        }
        if (find(hijo.begin(), hijo.end(), padre2.recorrido[i]) == hijo.end()) {
            hijo[j++] = padre2.recorrido[i];
        }
    }

    Individuo hijoIndividuo(ciudades);
    hijoIndividuo.recorrido = hijo;

    return hijoIndividuo;
}

void mutar(Individuo& individuo, const vector<Ciudad>& ciudades) {
    int indice1 = rand() % NUM_CIUDADES;
    int indice2 = rand() % NUM_CIUDADES;
    swap(individuo.recorrido[indice1], individuo.recorrido[indice2]);
    individuo.fitness = individuo.calcularFitness(ciudades);
}

pair<Individuo, Individuo> seleccionarPadres(const vector<Individuo>& poblacion) {
    int indice1 = rand() % poblacion.size();
    int indice2 = rand() % poblacion.size();
    if (poblacion[indice1].fitness > poblacion[indice2].fitness) {
        return make_pair(poblacion[indice1], poblacion[indice2]);
    }
    else {
        return make_pair(poblacion[indice2], poblacion[indice1]);
    }
}

Individuo ejecutarAlgoritmoGenetico(int poblacionSize, int generaciones, const vector<Ciudad>& ciudades, vector<double>& mejoresFitness) {
    srand(static_cast<unsigned int>(time(nullptr)));

    const int TAM_POBLACION = poblacionSize;
    const double PROBABILIDAD_MUTACION = 10;

    vector<Individuo> poblacion(TAM_POBLACION, Individuo(ciudades));

    Individuo mejorIndividuo(ciudades);
    double mejorFitness = mejorIndividuo.fitness;

    for (int generacion = 0; generacion < generaciones; ++generacion) {
        vector<Individuo> nuevaPoblacion;
        while (nuevaPoblacion.size() < TAM_POBLACION) {
            pair<Individuo, Individuo> padres = seleccionarPadres(poblacion);
            Individuo hijo = cruzar(padres.first, padres.second, ciudades);
            // Aplicar mutación con probabilidad PROBABILIDAD_MUTACION
            if ((rand() / static_cast<double>(RAND_MAX)) < PROBABILIDAD_MUTACION) {
                mutar(hijo, ciudades);
            }
            nuevaPoblacion.push_back(hijo);

            if (hijo.fitness > mejorFitness) {
                mejorIndividuo = hijo;
                mejorFitness = hijo.fitness;
            }
        }
        poblacion = nuevaPoblacion;

        mejoresFitness.push_back(1.0 / mejorFitness);
    }

    cout << "Mejor recorrido encontrado: ";
    for (int ciudad : mejorIndividuo.recorrido) {
        cout << ciudad << " ";
    }
    cout << "| Fitness: " << 1.0 / mejorFitness << endl;

    return mejorIndividuo;
}

int randInt(int min, int max) {
    return rand() % (max - min + 1) + min;
}

int main() {
    int poblacionSize = 20;
    int generaciones = 100;
    //srand(static_cast<unsigned int>(time(nullptr)));
    vector<Ciudad> ciudades(NUM_CIUDADES);
    for (int i = 0; i < NUM_CIUDADES; ++i) {
        ciudades[i].x = randInt(0, 100);
        ciudades[i].y = randInt(0, 100);
    }

    vector<double> mejoresFitness;

    Individuo mejorRecorrido = ejecutarAlgoritmoGenetico(poblacionSize, generaciones, ciudades, mejoresFitness);

    cout << "Mejor fitness encontrado: " << 1.0 / mejorRecorrido.fitness << endl;

    RenderWindow window(VideoMode(600, 600), "TSP");
    RenderWindow fitnessWindow(VideoMode(700, 700), "Fitness Evolution");
    Event event;
    window.setFramerateLimit(30);
    fitnessWindow.setFramerateLimit(30);

    // Cálculo de la escala para el gráfico de fitness
    double tope = mejoresFitness[0];
    double div = static_cast<double>(500 / tope);

    while (window.isOpen() || fitnessWindow.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) {
                window.close();
                fitnessWindow.close();
            }
        }

        window.clear();

        for (int i = 0; i < NUM_CIUDADES; i++) {
            CircleShape cir(10);
            cir.setPosition(static_cast<float>(ciudades[i].x) * 5, static_cast<float>(ciudades[i].y) * 5);
            window.draw(cir);
        }

        for (int i = 0; i < NUM_CIUDADES; i++) {
            int ciudadActual = mejorRecorrido.recorrido[i];
            int ciudadSiguiente = mejorRecorrido.recorrido[(i + 1) % NUM_CIUDADES];
            Vertex line[] =
            {
                Vertex(Vector2f(10 + static_cast<float>(ciudades[ciudadActual].x) * 5,10 + static_cast<float>(ciudades[ciudadActual].y) * 5)),
                Vertex(Vector2f(10 + static_cast<float>(ciudades[ciudadSiguiente].x) * 5,10 + static_cast<float>(ciudades[ciudadSiguiente].y) * 5))
            };
            window.draw(line, 2, Lines);
        }

        window.display();

        // Dibuja el gráfico de fitness
        fitnessWindow.clear();
        VertexArray ejeX(Lines, 2);
        ejeX[0].position = Vector2f(100, 100);
        ejeX[1].position = Vector2f(100, 600);
        fitnessWindow.draw(ejeX);

        VertexArray ejeY(Lines, 2);
        ejeY[0].position = Vector2f(600, 600);
        ejeY[1].position = Vector2f(100, 600);
        fitnessWindow.draw(ejeY);

        double divi = static_cast<double>(500 / generaciones);
        for (int i = 0; i < generaciones - 1; i += true) {
            VertexArray line(Lines, 2);
            //cout << 600 - mejoresFitness[i] * div << endl;
            line[1].position = Vector2f(100 + (i * divi), 600 - mejoresFitness[i] * div);
            line[0].position = Vector2f(100 + ((i + 1) * divi), 600 - mejoresFitness[i + 1] * div);
            fitnessWindow.draw(line);
        }

        fitnessWindow.display();
    }

    return 0;
}
