print(mud_name)
print(mud_description)
print(mud_connection_msg)
print("authors:\r\n" .. mud_credits)

connection_count = connection_count +1
print("You are connection number " .. connection_count .. " to connect since the last restart")
