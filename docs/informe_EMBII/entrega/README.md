# Sistema Automático de Clasificación de Tomates — Memoria Sistemas Embebidos II

## Estructura de archivos

```
entrega/
├── proyecto.tex              ← DOCUMENTO PRINCIPAL (editar aquí cambios)
├── bibliografia.bib          ← Referencias APA (NO tocar, agregar al .bib si necesita citas)
├── catolica_proyectos.cls    ← Clase LaTeX DCT (NO MODIFICAR)
├── gen_figuras.py            ← Script que generó PNG (solo referencia, no necesario en Overleaf)
├── Figuras/
│   ├── ucb-logo.png          ← Logo UCB (carátula)
│   ├── mec.png               ← Logo Mecatrónica (carátula)
│   ├── sis.png               ← Logo Sistemas (soporte)
│   ├── bmd.png               ← Logo Biomédica (soporte)
│   ├── pwm_10khz.png         ← Gráfica PWM (generada)
│   ├── ledc_duty.png         ← Gráfica LEDC (generada)
│   ├── cronograma_valvula.png ← Cronograma válvula (generada)
│   └── latencia_e2e.png      ← Presupuesto latencia (generado)
└── README.md                 ← Este archivo
```

## Pasos para compilar en Overleaf

### PASO 1: Descargar este ZIP desde tu PC

(El archivo estará en tu navegador como `entrega.zip`)

### PASO 2: Crear proyecto nuevo en Overleaf

1. Ve a **https://www.overleaf.com** e inicia sesión (crea cuenta si no tienes).
2. Click en **"New Project"** → **"Upload Project"**.
3. Selecciona el `entrega.zip` que descargaste.
4. Overleaf lo descomprimirá automáticamente en una carpeta con todos los archivos.

### PASO 3: Compilar el documento

1. En Overleaf, verifica que `proyecto.tex` sea el archivo raíz (debe aparecer en verde arriba).
2. Si no, haz click en el menú (hamburguesa, arriba a la izquierda) → **"Select Main File"** → elige `proyecto.tex`.
3. Click en **"Recompile"** (botón grande a la derecha).
4. En 30–60 segundos aparecerá el PDF compilado en el panel derecho.

**Nota:** La compilación usa `pdflatex` + `biber` (Overleaf lo hace automáticamente). Si ves errores de "biblatex", recarga la página (a veces tarda en actualizar la caché).

### PASO 4: Incorporar FOTOGRAFÍAS REALES

El documento tiene **espacios reservados** (recuadros grises) para las fotos que DEBEN tomar del prototipo. Para reemplazarlas:

1. **Toma las fotos** (ver lista abajo) y guárdalas como PNG/JPG.
2. **En Overleaf**, sube las imágenes: click en ícono de carpeta (arriba a la izquierda) → **"Files"** → **"Upload"** → selecciona la foto.
3. Overleaf las guarda en la carpeta raíz (o en `Figuras/` si las arrastras ahí).
4. En `proyecto.tex`, busca la línea `\fotopendiente{...}` (Buscar con Ctrl+F) que corresponda a esa foto.
5. Reemplaza la línea:
   ```latex
   % ANTES:
   \fotopendiente[6cm]{Esquemático y/o PCB de la consola...}
   
   % DESPUÉS:
   \includegraphics[width=0.8\textwidth]{./Figuras/mi_foto_consola.png}
   ```
6. Recompila (Ctrl+Enter o botón **"Recompile"**).

### PASO 5: Descargar el PDF final

1. Una vez compilado sin errores, haz click en el ícono de descarga (↓) en el panel del PDF.
2. Selecciona **"PDF"**.
3. Guárdalo como `MEMORIA_SISTEMAS_EMBEBIDOS_II.pdf` en tu PC.

---

## LISTA DE FOTOGRAFÍAS A TOMAR

Cada foto debe ser **clara, bien iluminada** y **nombrada descriptivamente**. Aquí está dónde va cada una en el documento:

| # | Foto | Ubicación en doc | Resolución mín. | Notas |
|---|------|------------------|-----------------|-------|
| 1 | **Montaje general**: banda + cámaras + zona de visión | Sección 1 (opción) | 1920×1440 | Vista lateral y cenital |
| 2 | **ESP32 Motor** cableado + BTS7960 + relé | Sección 4, después tabla pinout | 1600×1200 | Mostrar tierra común |
| 3 | **Osciloscopio PWM** en GPIO25 (10 kHz, 58.8% duty) | Figura 5 (reemplaza `\fotopendiente`) | 1024×768 | Captura real de sonda |
| 4 | **Consola LCD** encendida, mostrando pantalla de contadores | Sección 4, después Figura PCB | 1600×1200 | Mostrar texto en pantalla |
| 5 | **PCB Consola** (esquemático + placa física) | Figura 4, sección 4 | 1600×1200 | Exportar del KiCAD o fotografiar |
| 6 | **Monitor serial** mostrando "IP obtenida" + "Conectado broker" | Sección 4 (después pinout) | 1024×768 | Screenshot del PuTTY/minicom |
| 7 | **Terminal MQTT** (`mosquitto_sub` viendo tráfico en vivo) | Sección 5 (comunicaciones) | 1024×768 | Screenshot RPi con tópicos |
| 8 | **Cilindro neumático disparando** (expulsando fruto) | Sección 3 o 8 (anexo) | 1600×1200 | Video extraído o secuencia |
| 9 | **Tomates reales** (sanos y deteriorados, idealmente de Chocloca) | Sección 8 (caso real) | 1920×1440 | Sobre fondo blanco |

---

## Edición futura

Si necesitas cambiar contenido:

1. **Cambiar texto**: edita directamente en `proyecto.tex` en Overleaf (panel izquierdo).
2. **Cambiar bibliografía**: edita `bibliografia.bib` y ejecuta Ctrl+Enter para recompilar (Overleaf ejecuta `biber` automáticamente).
3. **Agregar secciones nuevas**: copia/pega una sección existente y modifica títulos y contenido.
4. **Cambiar figuras**: reemplaza los PNG en `Figuras/` o modifica los comandos `\includegraphics`.

---

## Solución de problemas

### **Error: "Package biblatex Error"**
→ Recarga la página. Overleaf descarga biblatex bajo demanda y a veces tarda.

### **Error: "File not found: Figuras/xxx.png"**
→ Verifica que las imágenes estén en `Figuras/` (carpeta visible en el panel de archivos) y que el nombre sea exacto (mayúsculas, extensión).

### **El PDF no se ve o está en blanco**
→ Busca mensajes de error en el panel "Logs" (abajo, en Overleaf). Copialos y revisa sintaxis LaTeX.

### **Quiero compilar sin Overleaf (local)**
→ Necesitas `pdflatex`, `biber`, `texlive-fonts-extra` instalados. En la raíz, ejecuta:
```bash
pdflatex proyecto.tex
biber proyecto
pdflatex proyecto.tex
pdflatex proyecto.tex
```
Genera `proyecto.pdf` al tercer pase.

---

## Contacto y referencias

- **Plantilla original**: Departamento de Ciencias de la Tecnología e Innovación, UCB San Pablo.
- **Documentación LaTeX**: https://www.overleaf.com/learn
- **Bibliografía APA**: https://www.overleaf.com/learn/latex/Bibliography_management_with_biblatex

¡Éxito con la defensa! 📄
