# RG projekat
Modeli i teksture preuzeti sa sledecih linkova:
- http://www.texturise.club
- https://www.cgtrader.com
- https://free3d.com

Model svetla i prostorije su modelirani samostalno pomocu Blender-a,
ostali su preuzeti sa linkova iznad.

Projekat obuhvata sledece oblasti:
1. Grupa A:
   - Cubemaps
2. Grupa B:
   - Normal mapping (trava i svi modeli osim modela svetla),
   Parallax mapping (trava i zidovi prostorije)
3. Osnovne oblasti:
    - Shaders
    - Textures
    - Affine transformation
    - Blinn-Phong Lighting (direkciono svetlo, tackasto svetlo, lampa)
    - Materials and lighting maps (difuzne, specularne)
    - Camera
    - Model loading and rendering
    - Blending (discard - na prozoru prostorije)
    - Face Culling (na modelima svetla)
# Uputstvo
1. `git clone https://github.com/nikollaa23/RG-projekat.git`
2. CLion -> Open -> path/to/my/project_base
3. Main se nalazi u src/main.cpp
4. Cpp fajlovi idu u src folder
5. Zaglavlja (h i hpp) fajlovi idu u include
6. Šejderi idu u folder shaders. `Vertex shader` ima ekstenziju `.vs`, `fragment shader` ima ekstenziju `.fs`
7. ALT+SHIFT+F10 -> project_base -> run
8. Komande tastature:
    - W,A,S,D - kretanje kamere,
    - L - ukljucivanje/iskljucivanje lampe,
    - F1 - ukljucivanje/iskljucivanje IMGui prozora

