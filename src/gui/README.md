Design considerations:

* Don't worry about animations or embedded drawing canvas.
* Draw objects to an sf::RenderTexture to avoid redrawing everything each frame.
    * If any object updates, redraw everything.
