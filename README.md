# OpenGL Flight Simulator

Projekt symulatora lotu, początkowo zakładający model samolotu, ostatecznie skupiający się na helikopterze zrealizowanym przy użyciu OpenGL i C++.

## Opis projektu

Oryginalna wizja obejmowała symulację lotu samolotu wraz z symulacją fizyki, jednak z powodu wyzwań technicznych przeszliśmy na model helikoptera. Obecna wersja oferuje podstawowe sterowanie ruchem i fizyką wirnika, a także wizualizację środowiska 3D w czasie rzeczywistym.

## Technologie i biblioteki

- **C++**
- **OpenGL** (rendering 3D)
- **GLFW** – okna i obsługa wejścia
- **GLEW** – zarządzanie rozszerzeniami OpenGL
- **GLM** – matematyka wektorowa i macierzowa

## Cechy

- Prosty model lotu helikoptera (pitch, roll, yaw, thrust)
- Obsługa kamery śledzącej z możliwością zmiany perspektywy
- Sterowanie przez klawiaturę i/lub joystick:
  - **W/S** – pochylanie przód/tył
  - **A/D** – pochylanie lewo/prawo
  - **Q/E** – obrót wokół osi pionowej (yaw)
  - **PageUp/PageDown** lub **F/C** – zmiana ciągu wirnika
- Sceneria: podłoże i prosty model miasta



