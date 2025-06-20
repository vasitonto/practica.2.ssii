#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <list>
#include <vector>
using namespace std;

 /**
   * Enumerado tipoR:
   * Un enumerado con los tipos de reglas que puede recibir el programa. Se usa en el tipo regla. 
   * Cuando la regla esté definida, tipoR contendrá el valor 0 en caso de que sea una conjunción, 
   * 1 si es una disyunción, y 2 si es una regla con un sólo antecedente. 
   * 
   */ 
enum tipoR{
        CONJ,
        DISY,
        SINGLE,
};


 /**
   * Tipo regla:
   * El tipo regla se utiliza para definir las reglas del sistema y consta de varios atributos;
   *  int numero: el nº de la regla, se utiliza como identificador.
   *  vector<string> antecedentes: un vector de string que contiene los antecedentes de la regla.
   *  tipoR tipo: contiene uno de los valores del enumerado tipoR.
   *  double fc: el factor de certeza con el que se cumple la regla
   *  double valor: contendrá el valor de dicha regla una vez calculada, 
   *    para realizar el resto de cálculos con más facilidad. 
   */
struct regla {
    int numero;
    vector<string> antecedentes;
    string consecuente;
    tipoR tipo; 
    double fc;  
    double valor;
};

// estas son variables globales que se utilizarán a lo largo del programa. 
// 
// map<string, double> BH: Un mapa que almacenará la base de hechos. 
//          La clave es un string que contiene el nombre del hecho, y el valor un double que contiene su factor de certeza.
// list<regla> BC: Una lista de elementos de tipo regla, almacenará la base de conocimiento. 
// string objetivo: El string objetivo se escribe en la función parseHechos() y guarda el objetivo inicial del motor. 
// int nh, nr: Los enteros nh y nr almacenan el número de hechos y el número de reglas respectivamente. 

map<string, double> BH;
list<regla> BC;
string objetivo;
int nh, nr;
set <int> CC;
set <string> nuevasMetas;

//Aquí se referencia la función calcular regla para poder referenciarla antes de su definición. 
double calcularRegla(regla r);


 /**
   * parseHechos(string fichero):
   * Esta función se llama desde el main al empezar la ejecución. Recibe el camino al fichero de la base de hechos
   * y los almacena en el mapa global BH. También recoge el valor del objetivo inicial y lo escribe en la variable 
   * global "objetivo". No devuelve nada ya que escribe en variables globales.  
   *
   * @param fichero: Es un string que almacena el camino al fichero
   */
void parseHechos(string fichero){
    ifstream bh;
    bh.open(fichero, ios::in);

    if (bh.is_open()){
        int nh = 0;
        bh >> nh;
        string linea;
        string subLin, subLin0;
        subLin="";
        subLin0="";
        size_t eq;    
        bh.ignore();
        char nwl = bh.peek();
        if(nwl=='\n') bh.ignore();

        for (int i = 0; i < nh; i++){
            getline(bh, linea);
            eq = linea.find(',');
            subLin = linea.substr(0, eq); 
            eq = linea.find('=');
            subLin0 = linea.substr(eq+1); 
            double valor = stof(subLin0);
            BH.insert(pair<string,double>(subLin, valor));
        }
        bh.ignore(9);
        bh >> objetivo;
    }
    return;
}

 /**
   * parseReglas(string fichero):
   * Tiene la misma función que parseHechos solo que escribe la base de conocimiento. Extrae los valores de cada
   * regla mediante streams. 
   *
   * @param fichero: es un camino al fichero de la base de conocimiento. 
   */
void parseReglas(string fichero){
    
    ifstream bc;
    bc.open(fichero, ios::in);
    if (bc.is_open()){
        //obtiene el nº de reglas
        bc >> nr;
        string linea = "";
        bc.ignore(256,'\n');
        for(int i = 0; i < nr; i++){
            getline(bc, linea);
            regla reg;
            stringstream estrim;
            string aux;
            reg.numero = i+1;
            estrim << linea;
            //coloca el stream justo antes del 1er elemento para almacenarlo
            estrim.ignore(6);
            estrim >> aux;
            reg.antecedentes.push_back(aux);

            //comprueba el caracter después del 1er antecedente para conocer el tipo de regla
            estrim >> aux;
            if (aux == "y") reg.tipo = CONJ;
            else if(aux == "o") reg.tipo = DISY;
            else if (aux.compare("Entonces") == 0) reg.tipo = SINGLE;
            bc.ignore();
            //despues de comprobar el tipo de función comienza a almacenar mas antecedentes
            //en el caso de que hubiera
            while (aux.compare("Entonces") != 0){
                estrim >> aux;
                reg.antecedentes.push_back(aux);
                estrim >> aux;
            }
            
            //cuando encuentra el "Entonces" sale del bucle y almacena el consecuente
            estrim >> aux;
            //hay que recortar el último caracter porque hay una coma justo después del nombre del consecuente 
            aux.pop_back();
            reg.consecuente = aux;
            estrim >> aux;
            reg.fc = stod(aux.substr(3));

            BC.push_back(reg);
        }
    }

    return;
    
}

/**
 * imprimeCC():
 * Esta función recorre y da formato al CC, y lo imprime por la salida estándar. 
*/
void imprimeCC(){
    cout << "\tCC = { ";
    for(int regla : CC){
        cout << "R" << regla << " ";
    }
    cout << "}" << endl;
}

/**
 * imprimeMetas():
 * Esta función recorre y da formato al conjunto de Nuevas Metas, y lo imprime por la salida estándar. 
*/
void imprimeMetas(){
    cout << "\tNuevas Metas = { ";
    for(string hecho : nuevasMetas){cout << hecho << " ";}
    cout << "}" << endl << endl;
}

 /**
   * motor(string o):
   * La función motor recibe como parámetro un hecho a calcular(string o) y devuelve su valor según 2 criterios:
   *    1: si el valor ya está en la BH lo devuelve directamente.
   *    2: si no está en la BH, recoge todas las reglas que lo tienen como consecuente, y calcula 
   *       el valor de cada una de ellas mediante llamadas a la función calculaRegla().
   *       Una vez se ha calculado el valor, comprueba si el hecho a calcular es consecuente en 1
   *       o más de 1 reglas, y realiza las operaciones necesarias en cada caso. 
   * 
   * @param string o: Un string que contiene el nombre del hecho que se quiere calcular. 
   */
double motor(string o){
    cout << "Queremos calcular \"" << o << "\"" << endl;
    //comprueba si el valor de "o" ya está calculado. Si lo está, lo devuelve.
    if(BH.count(o) == 1){
        cout << "\tEl valor ya esta almacenado: " << BH[o] << endl;
        nuevasMetas.erase(o);
        imprimeMetas();
        return BH[o];
    }
    //si no está calculado, realiza los cálculos pertinentes según ciertas condiciones.
    else{
        nuevasMetas.erase(o);
        imprimeMetas();

        // Almacena todas las reglas para las cuales "o" es consecuente en un vector de reglas, y 
        // el ID de la regla en el conjunto CC. 
        vector<regla> reglas;
        for (auto i : BC){  
            if(i.consecuente == o){
                //añade la regla al conjunto de reglas nuevas
                reglas.push_back(i);
                CC.insert(i.numero);
            }
        }
        imprimeCC();

        // También almacena todos los antecedentes de las nuevas reglas en el conjunto nuevasMetas.
        for(auto reg : reglas){
            for(auto ante : reg.antecedentes){
                nuevasMetas.insert(ante);
            }
        }

        imprimeMetas();

        // Para cada una de las reglas, llama a la función calcularRegla() y almacena el valor devuelto en 
        // el tipo regla del conjunto reglas para cada una de ellas. 
        // También elimina la regla del conjunto CC, ya que ha sido calculada. 
        for(long unsigned int i = 0; i < reglas.size(); i++){
            cout << "Activacion de la regla R" << reglas[i].numero <<": " << endl;
            CC.erase(reglas[i].numero);
            imprimeCC();
            reglas[i].valor = calcularRegla(reglas[i]);
            cout << "\tValor calculado para " << o << " por R" << reglas[i].numero << ": " << reglas[i].valor << endl << endl;
        }

        // Si sólo una regla tiene a "o" de consecuente, devuelve su valor.
        if(reglas.size() == 1){
            BH.insert(pair<string, double>(o, reglas[0].valor));
            return reglas[0].valor;
        }

        // Si más de una regla tiene a "o" como consecuente, realiza un bucle para calcular el valor
        // final del hecho que queremos obtener.

        else if(reglas.size() > 1){
            cout << "\tCaso 2: el valor " << o << " es consecuente en " << reglas.size() << " reglas:" << endl;
            for (auto reg : reglas){
                cout << "\t\tR" << reg.numero << " con valor " << reg.valor << endl;
            }
            
            // Para calcular el valor final de o se usan 2 floats auxiliares. En el primero se almacena el primer
            // valor del conjunto reglas, ya que el bucle comienza desde el 2º valor. En la segunda auxiliar se va 
            // guardando el valor calculado entre el valor calculado en la iteración anterior y el de la regla nueva.
            double faux = reglas[0].valor;
            double faux2 = 0.0;
            for(long unsigned int v = 1; v < reglas.size(); v++){
                cout << "\tCombinacion de R" << reglas[v-1].numero << " y R" << reglas[v].numero << " para calcular " << o << endl;
                cout << "\t"<< faux << " + " << reglas[v].valor;
                // Si ambos valores son positivos:
                if (faux >= 0 && reglas[v].valor >= 0){
                    faux2 = faux + reglas[v].valor *(1-faux);

                    cout << " * (1 - " << reglas[v].valor << ") = " << faux2 << endl;
                }
                // Si ambos valores son negativos:   
                else if (faux <= 0 && reglas[v].valor <= 0){
                    faux2 = faux + reglas[v].valor*(1+faux);

                    cout << " * (1 + " << reglas[v].valor << ") = " << faux2 << endl;
                }
                // Si los valores tienen signos diferentes:
                else {
                    faux2 = (faux + reglas[v].valor) / (1 - min(std::abs(faux), std::abs(reglas[v].valor)));
                    cout << " / (1 - min(|" << abs(faux) <<"|, |" << abs(reglas[v].valor) << "|)) = ";
                }
                cout << faux2 << endl;
                faux = faux2;
            }
            cout << "El valor calculado para " << o << " es: " << faux2 << endl << endl;
            return faux;
        }
        else{
            cout << "No se han encontrado reglas en las que " << o << " es el consecuente." << endl << "Se recomienda revisar el fichero BC.";
            exit(1);
        }
    }
}
//obtiene el valor de un hecho (string) que es pasado como parámetro.
//Si existe en la BH, devuelve el valor asignado, si no, lo calcula y lo añade a la BH interna
 /**
   * calcularRegla(regla r):
   * Esta función recibe una regla a calcular. Para calcularla comprueba el tipo de regla que es, 
   * y realiza las operaciones necesarias. Para cada antecedente hace una llamada a motor(),
   * que buscará en la BH para encontrar el hecho, y si no está lo calculará. 
   *
   * @param regla r: almacena la regla que se quiere calcular mediante un tipo regla que 
   * contiene todos los datos necesarios. 
   * 
   * @return Devuelve el valor double de la regla calculada.  
   */
double calcularRegla(regla r){
    //nueva meta: objetivo pasado como parámetro

    switch (r.tipo){
        // Tipo de regla 0: conjuncion -> busca el menor valor.
        // Después se termina de calcular con un Caso 3.
        case 0:{
            double fc = 1.0;
            double afc = 0.0;
            cout << "\tCaso 1, Conjuncion: Escoger el minimo valor entre los antecedentes." << endl;
            for(auto i : r.antecedentes){
                afc = motor(i);
                if (fc > afc) fc = afc;                  
            }
            double fcnuevo = r.fc * max(double(0), fc);
            cout << "\tCaso 3 por conjuncion: " << fc << ", " << r.fc << "; FC(R" << r.numero <<") = " << r.fc <<" * max(0, " << fc << ") = " << fcnuevo << endl;
            return fcnuevo;
        }
            break;
        
        // Tipo 1: disyuncion -> busca el mayor valor.
        // Se completa con un Caso 3.
        case 1:{
            double fc = 0;
            double afc = 0;
            cout << "\tCaso 1, Disyuncion: Escoger el maximo valor entre los antecedentes." << endl;
            for(auto i : r.antecedentes){
                afc = motor(i);
                if (fc < afc) fc = afc;                 
            }
            double fcnuevo = r.fc * max(double(0), fc);
            cout << "\tCaso 3 por disyuncion: " << fc << ", " << r.fc << "; FC(R" << r.numero <<") = " << r.fc <<" * max(0, " << fc << ") = " << fcnuevo << endl;
            return fcnuevo;
        }
            break;

        // Tipo 2: regla individual. Basta con calcular el Caso 3 directamente.
        case 2:{
            //fc de la regla * max{0, FC antecedente}
            double fcant = motor(r.antecedentes[0]);
            double fcnuevo = r.fc * max(double(0), fcant);
            cout << "\tCaso 3 por regla unitaria: " << fcant << ", " << r.fc << "; FC(R" << r.numero <<") = " << r.fc <<" * max(0, " << fcant << ") = " << fcnuevo << endl;
            return fcnuevo; 
        }
            break;
        
        default:
            return 0;
            break;
    }
}


 /**
   * Función main
   * @param char const *argv[]: Los argumentos de programa, recibe los ficheros de la base de conocimiento
   *    y la base de hechos. Primero hace las llamadas a funciones que leen las bases de hechos, de conocimiento y el objetivo.
   *    Después llama a la función motor(), y escribe por salida estándar el valor que devuelve, que será el valor del 
   *    objetivo final. 
   */

int main(int argc, char const *argv[]){
    
    if (argc != 3) {
        cerr << "Uso correcto: se debe incluir el camino al fichero BC y al fichero BH en ese orden al llamar al programa. " << endl;
        cerr << "El comando debe quedar asi: ./\"nombre del ejecutable\" \"camino_BC\" \"camino_BH\"" << endl;
        exit(1);
    }

    //debug
    // parseReglas("./pruebas/BC-1.txt");
    // parseHechos("./pruebas/BH-1.txt");
    // map<string, double> BH2 = BH;
    // list<regla> BC2 = BC;
    // string objetivo2 = objetivo;

    parseReglas(argv[1]);
    parseHechos(argv[2]);

    //comprobación de errores.
    if(BC.size() == 0){
        cerr << "No se ha podido leer el fichero BC correctamente. Asegurate de que la ruta esta bien escrita." << endl;
        cerr << "Uso correcto: ./\"nombre del ejecutable\" \"camino_BC\" \"camino_BH\"" << endl;
        exit(1);
    }
    else if(BH.size() == 0){
        cerr << "No se ha podido leer el fichero BH correctamente. Asegúrate de que la ruta esta bien escrita." << endl;
        cerr << "Uso correcto: ./\"nombre del ejecutable\" \"camino_BC\" \"camino_BH\"" << endl;
        exit(1);
    }
    else cout << "Los ficheros se han abierto correctamente." << endl;
    
    cout << "BC usada: " << argv[1] << endl << "BH usada: " << argv[2] << endl << "Objetivo: " << objetivo << endl;
    cout << "Comenzamos razonamiento: " << endl << endl;
    
    double sol = double(motor(objetivo));
    cout << "La solución final es " << sol << endl;

    return 0;
}
