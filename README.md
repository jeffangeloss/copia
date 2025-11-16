# Sensores Envio — Guia rapida

Este repositorio contiene el firmware del ESP32, el panel web PaginaSemaforos y el servidor proxy que vincula ambos componentes.

## Preparacion inicial

1. Clona el repositorio y entra en la carpeta raiz:
   ```bash
   git clone https://github.com/jeffangeloss/sensores-envio.git
   cd sensores-envio
   ```
2. Instala las dependencias del dashboard (solo la primera vez):
   ```bash
   npm --prefix PaginaSemaforos install
   ```
3. Opcional: instala los atajos npm de la raiz si prefieres usarlos:
   ```bash
   npm install
   ```

## Como actualizar tu copia local

Si ya tienes el proyecto en `C:\Users\<usuario>\OneDrive\Escritorio\sensores-envio`, puedes sincronizarlo con los ultimos cambios con estos pasos:

1. Asegurate de no tener cambios locales pendientes o hazles commit.
2. Desde la raiz del proyecto, trae la ultima version del repositorio remoto:
   ```bash
   git pull
   ```
3. Vuelve a instalar dependencias si hubo cambios en los manifiestos:
   ```bash
   npm install                       # scripts de atajo en la raiz (opcional)
   npm --prefix PaginaSemaforos install
   ```
4. Reconstruye la aplicacion web para actualizar `PaginaSemaforos/dist`:
   ```bash
   npm run build
   ```

## Comandos de uso frecuente

* Levantar el entorno de desarrollo del dashboard (Vite):
  ```bash
  npm run dev
  ```
* Levantar Vite expuesto en la red local:
  ```bash
  npm run dev:host
  ```
* Servir la build y levantar el proxy HTTP hacia el ESP32:
  ```bash
  npm run proxy -- --esp32 http://10.122.132.XXX
  ```
  Ajusta los ultimos octetos segun la IP asignada por tu hotspot.

## Mas informacion

* El firmware del ESP32 esta en `Inicio_Semaforos/Inicio_Semaforos.ino`.
* El dashboard React (Vite + Tailwind) vive en `PaginaSemaforos/`.
* El servidor proxy y de archivos estaticos es `Servidor.py`.

## Base de datos para autenticacion

Para facilitar las pruebas del modulo de login sin depender de un backend externo, se agrego una base de datos local en `PaginaSemaforos/src/lib/localUserDatabase.ts`. Este archivo contiene cuatro cuentas administrativas con contrasenas encriptadas mediante una funcion hash ligera basada en FNV-1a. El contexto de autenticacion (`src/contexts/AuthContext.tsx`) consulta primero esta base antes de delegar el inicio de sesion a Supabase, por lo que siempre tendras credenciales conocidas para validar la interfaz.

### Usuarios disponibles

| Email | Rol | Area | Contrasena |
|-------|-----|------|------------|
| gabylolicious@gmail.com | Administrador | Coordinacion General | Gabriela12 |
| jjjangelosss@gmail.com | Administrador | Coodinacion Tecnica | gabyjeff1617!!! |
| joaquin.castillo@gmail.com | Administrador | Desarrollo de Software | Joaquin2025 |
| andrea.barro@gmail.com | Administrador | Arquitectura de Software | Admin#2025 |

## Tutorial remoto para Gaby

1. Enciende el ESP32 y anota la IP que muestre (ej. `192.168.18.197`).
2. Desde tu PC anfitrion ejecuta:
   ```powershell
   npm run proxy -- --esp32 http://192.168.18.197
   ```
   Este comando compila la SPA, lanza `Servidor.py` en el puerto 8080 y deja configurado el proxy hacia la IP real del micro.
3. Abre otra terminal y expone el servicio por ngrok usando el subdominio reservado:
   ```bash
   ngrok http --domain=semiobjective-mazie-semestrial.ngrok-free.dev 8080
   ```
4. Comparte con Gaby la URL `https://semiobjective-mazie-semestrial.ngrok-free.dev/dashboard` y las credenciales `gabylolicious@gmail.com / Gabriela12`.
5. Desde su casa, Gaby inicia sesion, verifica que la tarjeta "Conexion" indique "Conectado" y presiona **Iniciar** para controlar el ciclo del ESP32.

## Evaluacion rapida de un Gateway

El proyecto ya incluye `Servidor.py`, que actua como servidor de archivos estaticos y proxy hacia el ESP32. Este componente puede evolucionar hacia un Gateway de autenticacion con las siguientes mejoras:

1. **Unificacion de identidad**: mover la verificacion de usuarios (hoy en el frontend) al servidor, exponiendo endpoints REST (`/auth/login`, `/auth/logout`) que consuman la misma base de datos o un servicio como Supabase.
2. **Politicas por rol**: el Gateway puede inyectar encabezados o claims JWT con el rol (`Operador`, `Administrador`, etc.) para que el firmware o futuros microservicios habiliten/denieguen operaciones sensibles.
3. **Registro y auditoria**: al centralizar el trafico HTTP seria posible almacenar logs de ingreso, cambios de configuracion o comandos enviados al ESP32, mejorando el seguimiento de incidentes.

Estas capacidades se pueden incorporar progresivamente sin romper el flujo actual: basta con extender `Servidor.py` con un modulo ligero (por ejemplo FastAPI o Flask) y apuntar la SPA a esos endpoints antes de reenviar comandos al microcontrolador.

Con estos pasos puedes mantener tu entorno local alineado con los cambios realizados en el repositorio y relanzar los servicios necesarios.
