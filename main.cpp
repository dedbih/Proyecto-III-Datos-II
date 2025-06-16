#include <iostream>   // Para entrada/salida estándar (cout, cin)
#include <vector>     // Para usar contenedores dinámicos de tipo vector
#include <stdexcept>  // Para manejo de excepciones

// Prototipos de funciones para mejor organización
std::vector<char> calcularParidad(const std::vector<std::vector<char>>& bloquesDatos);
std::vector<char> reconstruirBloque(const std::vector<std::vector<char>>& bloquesDisponibles,
                                  const std::vector<char>& paridad);
void mostrarBloque(const std::vector<char>& bloque, const std::string& etiqueta);

int main() {
    try {
        // ========================================================================
        // 1. Configuración inicial - Definimos 3 bloques de datos de ejemplo, dentro de un vector
        // ========================================================================
        std::vector<std::vector<char>> bloques = {
            {'A', 'B', 'C', 'D'},  // Bloque 1 - 4 bytes de datos
            {'E', 'F', 'G', 'H'},  // Bloque 2 - 4 bytes de datos
            {'I', 'J', 'K', 'L'}   // Bloque 3 - 4 bytes de datos
        };

        // Mostrar bloques originales
        std::cout << "\n=== Configuracion Inicial ===" << std::endl;
        for (size_t i = 0; i < bloques.size(); ++i) {
            mostrarBloque(bloques[i], "Bloque " + std::to_string(i+1));
        }

        // ========================================================================
        // 2. Cálculo del bloque de paridad (usando XOR)
        // ========================================================================
        std::vector<char> paridad = calcularParidad(bloques);
        mostrarBloque(paridad, "\nBloque de paridad calculado");

        // ========================================================================
        // 3. Simulación de fallo - Eliminamos el Bloque 2 (índice 1)
        // ========================================================================
        std::cout << "\n=== Simulando falla del Bloque 2 ===" << std::endl;
        std::vector<std::vector<char>> bloquesRestantes = {bloques[0], bloques[2]};

        // Mostrar bloques disponibles después del fallo
        mostrarBloque(bloquesRestantes[0], "Bloque 1 disponible");
        mostrarBloque(bloquesRestantes[1], "Bloque 3 disponible");

        // ========================================================================
        // 4. Reconstrucción del bloque perdido
        // ========================================================================
        std::vector<char> bloqueRecuperado = reconstruirBloque(bloquesRestantes, paridad);

        // ========================================================================
        // 5. Verificación de resultados
        // ========================================================================
        bool exito = bloqueRecuperado == bloques[1];
        std::cout << "\n=== Resultados de Reconstrucción ===" << std::endl;
        std::cout << "Recuperación " << (exito ? "EXITOSA" : "FALLIDA") << std::endl;

        mostrarBloque(bloques[1], "Bloque original (perdido)");
        mostrarBloque(bloqueRecuperado, "Bloque recuperado");

    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// ==============================================================================
// Función: calcularParidad
// Descripción: Calcula el bloque de paridad usando operación XOR sobre múltiples bloques
// ==============================================================================
std::vector<char> calcularParidad(const std::vector<std::vector<char>>& bloquesDatos) {
    // Verificar que hay bloques para procesar
    if (bloquesDatos.empty()) {
        throw std::invalid_argument("No se proporcionaron bloques de datos");
    }

    // Todos los bloques deben tener el mismo tamaño
    size_t tamanoBloque = bloquesDatos[0].size();
    for (const auto& bloque : bloquesDatos) {
        if (bloque.size() != tamanoBloque) {
            throw std::invalid_argument("Todos los bloques deben tener el mismo tamaño");
        }
    }

    // Inicializar vector de paridad con ceros
    std::vector<char> paridad(tamanoBloque, 0);

    // Calcular paridad byte a byte usando XOR
    for (const auto& bloque : bloquesDatos) {
        for (size_t i = 0; i < tamanoBloque; ++i) {
            paridad[i] ^= bloque[i];  // Operación XOR acumulativa
        }
    }

    return paridad;
}

// ==============================================================================
// Función: reconstruirBloque
// Descripción: Reconstruye un bloque perdido usando los bloques disponibles + paridad
// ==============================================================================
std::vector<char> reconstruirBloque(const std::vector<std::vector<char>>& bloquesDisponibles,
                                  const std::vector<char>& paridad) {
    //se iguala el bloque de paridad con el bloque recuperado
    //se compara e iguala el bloque recuperado a la operación xor con bloque[i]

    // Verificar parámetros de entrada
    if (paridad.empty()) {
        throw std::invalid_argument("El bloque de paridad está vacío");
    }

    size_t tamanoBloque = paridad.size();
    std::vector<char> bloqueRecuperado(tamanoBloque, 0);

    // Paso 1: Copiar la paridad como base para la reconstrucción
    for (size_t i = 0; i < tamanoBloque; ++i) {
        bloqueRecuperado[i] = paridad[i];
    }

    // Paso 2: Aplicar XOR con los bloques disponibles
    for (const auto& bloque : bloquesDisponibles) {
        // Verificar que el bloque tenga el tamaño correcto
        if (bloque.size() != tamanoBloque) {
            throw std::invalid_argument("Tamaño de bloque incompatible");
        }

        for (size_t i = 0; i < tamanoBloque; ++i) {
            bloqueRecuperado[i] = bloqueRecuperado[i] ^ bloque[i]; // XOR operation
        }
    }

    return bloqueRecuperado;
}



// ==============================================================================
// Función: mostrarBloque
// Descripción: Muestra el contenido de un bloque en formato legible
// ==============================================================================
void mostrarBloque(const std::vector<char>& bloque, const std::string& etiqueta) {
    std::cout << etiqueta << " [";

    // Mostrar valores en formato char (si es imprimible) o como número
    for (size_t i = 0; i < bloque.size(); ++i) {
        if (i > 0) std::cout << ", ";

        // Mostrar caracteres imprimibles directamente, otros como ASCII
        if (isprint(bloque[i])) {
            std::cout << "'" << bloque[i] << "'";
        } else {
            std::cout << static_cast<int>(bloque[i]);
        }
    }
    std::cout << "]" << std::endl;
}