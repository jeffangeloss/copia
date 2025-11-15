# Sensores Envío — Guía rápida

Este repositorio contiene el firmware del ESP32, el panel web PaginaSemaforos y el servidor proxy que vincula ambos componentes.

## Preparación inicial

1. Clona el repositorio y entra en la carpeta raíz:
   ```bash
   git clone https://github.com/jeffangeloss/sensores-envio.git
   cd sensores-envio
   ```
2. Instala las dependencias del dashboard (solo la primera vez):
   ```bash
   npm --prefix PaginaSemaforos install
   ```
3. Opcional: instala los atajos npm de la raíz si prefieres usarlos:
   ```bash
   npm install
   ```

## Cómo actualizar tu copia local

Si ya tienes el proyecto en `C:\Users\<usuario>\OneDrive\Escritorio\sensores-envio`, puedes sincronizarlo con los últimos cambios con estos pasos:

1. Asegúrate de no tener cambios locales pendientes o hazles commit.
2. Desde la raíz del proyecto, trae la última versión del repositorio remoto:
   ```bash
   git pull
   ```
3. Vuelve a instalar dependencias si hubo cambios en los manifiestos:
   ```bash
   npm install                       # scripts de atajo en la raíz (opcional)
   npm --prefix PaginaSemaforos install
   ```
4. Reconstruye la aplicación web para actualizar `PaginaSemaforos/dist`:
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
  Ajusta los últimos octetos según la IP asignada por tu hotspot.

## Más información

* El firmware del ESP32 está en `Inicio_Semaforos/Inicio_Semaforos.ino`.
* El dashboard React (Vite + Tailwind) vive en `PaginaSemaforos/`.
* El servidor proxy y de archivos estáticos es `Servidor.py`.

## Base de datos para autenticación

Para facilitar las pruebas del módulo de login sin depender de un backend externo, se agregó una base de datos local en `PaginaSemaforos/src/lib/localUserDatabase.ts`. Este archivo contiene diez cuentas preconfiguradas (operadores, administradores e invitados) con contraseñas encriptadas mediante una función hash ligera basada en FNV-1a. El contexto de autenticación (`src/contexts/AuthContext.tsx`) consulta primero esta base antes de delegar el inicio de sesión a Supabase, por lo que siempre tendrás credenciales conocidas para validar la interfaz.

### Usuarios disponibles

| Email                            | Rol            | Área                 | Contraseña    |
|----------------------------------|----------------|----------------------|---------------|
| ana.ramirez@trafico.local        | Operador       | Centro Histórico     | Semaforo#01   |
| ricardo.sosa@trafico.local       | Operador       | Zona Norte           | Semaforo#02   |
| marcela.arias@trafico.local      | Operador       | Zona Industrial      | Semaforo#03   |
| fernando.lagos@trafico.local     | Operador       | Zona Universitaria   | Semaforo#04   |
| claudia.vargas@trafico.local     | Operador       | Zona Sur             | Semaforo#05   |
| gustavo.pena@trafico.local       | Operador       | Anillo Periférico    | Semaforo#06   |
| maria.ortega@trafico.local       | Administrador  | Coordinación General | Admin#2024    |
| hector.paredes@trafico.local     | Administrador  | Seguridad Vial       | Admin#2025    |
| inspectora.navarro@trafico.local | Invitado       | Supervisión          | Invitado#1    |
| ingeniero.salas@trafico.local    | Invitado       | Infraestructura      | Invitado#2    |

## Evaluación rápida de un Gateway

El proyecto ya incluye `Servidor.py`, que actúa como servidor de archivos estáticos y proxy hacia el ESP32. Este componente puede evolucionar hacia un Gateway de autenticación con las siguientes mejoras:

1. **Unificación de identidad**: mover la verificación de usuarios (hoy en el frontend) al servidor, exponiendo endpoints REST (`/auth/login`, `/auth/logout`) que consuman la misma base de datos o un servicio como Supabase.
2. **Políticas por rol**: el Gateway puede inyectar encabezados o claims JWT con el rol (`Operador`, `Administrador`, etc.) para que el firmware o futuros microservicios habiliten/denieguen operaciones sensibles.
3. **Registro y auditoría**: al centralizar el tráfico HTTP sería posible almacenar logs de ingreso, cambios de configuración o comandos enviados al ESP32, mejorando el seguimiento de incidentes.

Estas capacidades se pueden incorporar progresivamente sin romper el flujo actual: basta con extender `Servidor.py` con un módulo ligero (por ejemplo FastAPI o Flask) y apuntar la SPA a esos endpoints antes de reenviar comandos al microcontrolador.

Con estos pasos puedes mantener tu entorno local alineado con los cambios realizados en el repositorio y relanzar los servicios necesarios.
