import sys

vals=[]
ouput=[]
dit_mems= {}
dit_insts={}
counter_vals =0
counter_instructions=0

with open(sys.argv[1], "r") as file:

    for line in file:
        line = line.split('#')[0]

        line = line.strip()

        if line == "" :
            continue

        if line.startswith('.'):
            parts = line[1:].split(':')

            if len(parts)==2:
                label = parts[0].strip()
                values = parts[1].split()

                dit_mems[label] = counter_vals
                counter_vals += len(values)
                vals.extend(values)

                continue

        elif ':' in line:
            line = line.split(':')

            dit_insts[line[0].strip()]=counter_instructions
            
            if line[1].strip()=="":
                continue

            else:
                
                counter_instructions+=1
                
        else:

            counter_instructions+=1

    file.seek(0)

    for line in file:

        line = line.split('#')[0]

        line = line.strip()

        if line == "" :
            continue
        if line.startswith('.'):
            continue

        elif ':' in line:
            line = line.split(':')
            
            if line[1].strip()=="":
                continue

            else:
                line = line[1]


        if 'lw' in line or 'sw' in line:

            parts = line.split()
            sub_parts = parts[2].split('(')

            mem_label = sub_parts[0].strip()

            if mem_label in dit_mems:
                res= parts[0].strip() + ' ' + parts[1].strip() + ' '+ str(dit_mems[mem_label]) + '('+ sub_parts[1]
            else:
                res= parts[0].strip() + ' ' + parts[1].strip() + ' '+ mem_label + '('+ sub_parts[1]

            ouput.append(res)
        
        elif 'beq' in line or 'bne' in line or 'blt' in line or 'ble' in line:
            parts = line.split()

            inst_label = parts[3].strip()

            if inst_label in dit_insts:
                res = parts[0].strip() + ' ' + parts[1].strip() + ' ' + parts[2].strip() + ' ' + str(dit_insts[inst_label])
            
            else:
                res = parts[0].strip() + ' ' + parts[1].strip() + ' ' + parts[2].strip() + ' ' + inst_label

            ouput.append(res)

        elif "j " in line:
            parts = line.split()

            inst_label = parts[1].strip()

            if inst_label in dit_insts:
                res = parts[0].strip() + " " + str(dit_insts[inst_label])
            else:
                res = parts[0].strip() + " " + inst_label

            ouput.append(res)
        
        else:
            ouput.append(line)


with open(sys.argv[2], 'w') as file:

    v = len(vals)
    if v!=0:
        file.write(".data : ")
        for i in range(v-1):
            file.write(str(vals[i])+ " ")

        file.write(str(vals[v-1]) + "\n")
    
    o = len(ouput)

    if o!=0:
        for i in range(o - 1):
            file.write(ouput[i] + "\n")
        
        file.write(ouput[o-1] + "\n")
        
        

