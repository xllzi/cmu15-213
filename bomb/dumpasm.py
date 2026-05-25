import lldb


def dumpasm(debugger, command, result, internal_dict):
    output_file = command.strip()

    if not output_file:
        result.PutCString("Usage: dumpasm <output_file>")
        return

    target = debugger.GetSelectedTarget()

    if not target:
        result.PutCString("No target selected.")
        return

    try:
        with open(output_file, "w") as out:
            for module in target.module_iter():
                out.write(f"\n######## MODULE: {module.file.basename} ########\n")

                for symbol in module:
                    if symbol.GetType() != lldb.eSymbolTypeCode:
                        continue

                    name = symbol.GetName()

                    start_addr = symbol.GetStartAddress()
                    end_addr = symbol.GetEndAddress()

                    if not start_addr.IsValid() or not end_addr.IsValid():
                        continue

                    start_load = start_addr.GetLoadAddress(target)
                    end_load = end_addr.GetLoadAddress(target)

                    if start_load == lldb.LLDB_INVALID_ADDRESS:
                        continue

                    if end_load == lldb.LLDB_INVALID_ADDRESS:
                        continue

                    out.write(f"\n===== FUNCTION: {name} =====\n")

                    cmd = (
                        f"disassemble "
                        f"--start-address {start_load} "
                        f"--end-address {end_load}"
                    )

                    res = lldb.SBCommandReturnObject()

                    debugger.GetCommandInterpreter().HandleCommand(cmd, res)

                    text = res.GetOutput()

                    if text:
                        out.write(text)
                        out.write("\n")

        result.PutCString(f"Assembly written to: {output_file}")

    except Exception as e:
        result.PutCString(f"Error: {e}")


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
        "command script add -f dumpasm.dumpasm dumpasm"
    )

