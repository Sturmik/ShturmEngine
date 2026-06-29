-- This is a Lua global variable
some_variable = 7 * 6
user_name = "Maximiliano"

print("Hello "..user_name)

if some_variable == 42 then
	print("The value of the variable is " ..some_variable )
end

-- This is a Lua table that contain values for the initialization of our game engine
config = {
	title = "Shturm Engine",
	fullscreen = false,
	resolution = { 
		width = 800,
		height = 600
	}
}

-- This is a Lua function that calculates the factorial of a given number
function factorial(n)
	if (n <= 1) then
		return 1
	end
	return n * factorial(n - 1)
end

-- Lua can apply C++ native function
print("The cube of the number 3 is equal to "..cube(3))