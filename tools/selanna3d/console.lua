-- Lua console

con = {}

con.lines = {}       -- output history
con.history = {}     -- input history
con.histIdx = 0      -- 0 = not browsing
con.input = ""

local MAX_LINES = 200

function con.print(...)
	local parts = {}
	for i = 1, select('#', ...) do
		parts[i] = tostring(select(i, ...))
	end
	local line = table.concat(parts, "\t")
	for l in (line .. "\n"):gmatch("([^\n]*)\n") do
		table.insert(con.lines, l)
	end
	while #con.lines > MAX_LINES do
		table.remove(con.lines, 1)
	end
end

function con.exec(s)
	con.print("> " .. s)
	-- try as expression first (prepend 'return')
	local chunk, err = load("return " .. s, "=console", "t", _ENV)
	if not chunk then
		-- fall back to statement
		chunk, err = load(s, "=console", "t", _ENV)
	end
	if not chunk then
		con.print("Error: " .. err)
		return
	end
	local oldprint = print
	print = con.print
	local results = table.pack(pcall(chunk))
	print = oldprint
	local ok = table.remove(results, 1)
	if not ok then
		con.print("Error: " .. tostring(results[1]))
	elseif results.n > 0 then
		local parts = {}
		for i = 1, results.n do
			parts[i] = tostring(results[i])
		end
		con.print(table.concat(parts, "\t"))
	end
end

local scrollToBottom = false

function con.gui()
--	ImGui.Begin("Console")

	local w, h = ImGui.GetContentRegionAvail()
	ImGui.BeginChild("output", 0, h - 28, true)
	for _, line in ipairs(con.lines) do
		ImGui.TextUnformatted(line)
	end
	if scrollToBottom then
		ImGui.SetScrollHereY(1.0)
		scrollToBottom = false
	end
	ImGui.EndChild()

	ImGui.PushItemWidth(-1)
	local flags = ImGuiInputTextFlags.EnterReturnsTrue
	local newInput, submitted = ImGui.InputText("##input", con.input, 256, flags)
	con.input = newInput
	if submitted and con.input ~= "" then
		table.insert(con.history, 1, con.input)
		con.histIdx = 0
		con.exec(con.input)
		con.input = ""
		scrollToBottom = true
		ImGui.SetKeyboardFocusHere(-1)
	end
	ImGui.PopItemWidth()

--	ImGui.End()
end

-- expose con.print as a global for convenience
conprint = con.print
