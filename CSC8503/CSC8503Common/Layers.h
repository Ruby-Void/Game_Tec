// Define Layers
#define LAYER_ZERO		(1 << 0) // Default Layer		- Includes Floor
#define LAYER_ONE		(1 << 1) // Enviroment Layer	- Includes Sphere and Cube
#define LAYER_TWO		(1 << 2) // Goose Layer			- Includes Goose
#define LAYER_THREE		(1 << 3) // Apple Layer			- Includes Apples
#define LAYER_FOUR		(1 << 4) // ParkKeeper Layer	- Includes ParkKeeper
#define LAYER_FIVE		(1 << 5) // Character Layer		- Includes Character
#define LAYER_SIX		(1 << 6) // UI
#define LAYER_SEVEN		(1 << 7) // To Be Used

// Define Masks
#define MASK_ACTIVE_LAYERS	(LAYER_ZERO | LAYER_ONE | LAYER_TWO | LAYER_THREE | LAYER_FOUR | LAYER_FIVE)