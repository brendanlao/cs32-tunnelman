# Tunnelman
Tunnelman is the final project for CS32 at UCLA, which incorporates OOP fundamentals such as inheritance and polymorphism.  The purpose if the game was to find all the barrels of oil in a tunnel map, and this was hindered by the presence of protestors who could prevent the tunnel man from reaching the barrels of oil.  The tunnel man was also able to interact with various consumables and game objects such as boulders, radars, and gold nuggets, which had their own corresponding logic. 

# Reflection
The most difficult part was figuring out how to manage organization for the files and determining what classes each class should inherit from.  For example, the tunnel man and protesters inherited from the "Actor" class since they had similar functionalities whereas boulders, oil barrels, and golden nuggets inherited from the "Distributables" class, which referred to objects distributed throughout the map when the game starts.  Along this same line, it was also a challenge to figure out which variables were unique to each class.  More specifically, all actors had an hp attribute, but not all actors had an inventory attribute, so the inventory attribute was only designated to the tunnelman class.   Overall, this project served as an effective introduction to OOP, providing hands-on experience in designing and implementing classes, understanding inheritance, and managing the relationships and interactions between various objects within a complex system.
