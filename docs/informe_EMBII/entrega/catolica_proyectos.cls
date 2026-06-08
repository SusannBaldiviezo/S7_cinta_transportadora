\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{catolica_proyectos}[2026/03/18 Clase DCT UCB]

% --- Herencia ---
\LoadClass[12pt,letterpaper,oneside]{article}

% --- Paquetes ---
\RequirePackage[utf8]{inputenc}
\RequirePackage[spanish, es-tabla]{babel}
\RequirePackage{graphicx}
\RequirePackage{xcolor}
\RequirePackage{geometry}
\RequirePackage{titlesec}
\RequirePackage{etoolbox}
\RequirePackage{parskip}

% --- Bibliografía APA 7ma ---
\RequirePackage{csquotes}
\RequirePackage[
backend=biber,
style=apa,
sortcites=true,
sorting=nyt
]{biblatex}

% --- Configuración de Márgenes Base ---
\geometry{
	letterpaper,
	top=2.5cm,
	bottom=2.5cm,
	left=3cm,
	right=2.5cm
}

% --- Variables ---
\newcommand{\titulo}[1]{\def\@titulo{#1}}
\newcommand{\materia}[1]{\def\@materia{#1}}
\newcommand{\carrera}[1]{\def\@carrera{#1}}
\newcommand{\autor}[1]{\def\@autor{#1}}
\newcommand{\docente}[1]{\def\@docente{#1}}
\newcommand{\semestre}[1]{\def\@semestre{#1}}

% --- Entorno Consigna ---
\newenvironment{consigna}[1]{
	\color{#1}\itshape\small
	\begin{quote}
		\textbf{Guía DCT:} 
	}{
	\end{quote}
	\normalcolor
}

% --- Redefinición de Carátula ---
\renewcommand{\maketitle}{
	% Cambiamos márgenes solo para la carátula
	\newgeometry{top=2cm, bottom=2.5cm, left=2.5cm, right=2.5cm}
	
	\begin{titlepage}
		\centering
		
		% Logo UCB
		\includegraphics[height=5cm]{Figuras/ucb-logo.png}\par
		\vspace{1cm}
		
		{\scshape\Large Universidad Católica Boliviana "San Pablo"\par}
		\vspace{0.3cm}
		{\scshape\large Departamento de Ciencias de la Tecnología e Innovación\par}
		\vspace{0.3cm}
		{\scshape\large Ingeniería \@carrera \par}
		
		\vspace{1.5cm}
		{\huge\bfseries \@titulo \par}
		
		\vspace{1.5cm}
		
		% --- Lógica robusta para el logo de carrera ---
		\ifdefstring{\@carrera}{Mecatrónica}{
			\includegraphics[width=5cm]{Figuras/mec.png}\par
		}{
			\ifdefstring{\@carrera}{Sistemas}{
				\includegraphics[width=5cm]{Figuras/sis.png}\par
			}{
				% Si no es ninguna de las anteriores, por defecto es Biomédica
				\includegraphics[width=5cm]{Figuras/bmd.png}\par
			}
		}
		
		\vfill
		
		\begin{flushleft}
			\large
			\textbf{Materia:} \@materia \\
			\textbf{Estudiante(s):} \@autor \\ % Agregamos el (s)
			\textbf{Docente:} \@docente \\
			\textbf{Semestre:} \@semestre
		\end{flushleft}
		
		\vspace{0.5cm}
		{\large Bolivia, \today \par}
	\end{titlepage}
	
	% Restauramos los márgenes de 3cm para el resto del documento
	\restoregeometry
}