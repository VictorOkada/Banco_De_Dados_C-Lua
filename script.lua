function validarCPF (tipo, CPF)

  res = string.match(CPF, "(%d+.%d+.%d+-%d+)")
  return res
  -- if string.match(tipo, "DATA") and res= nil then
  --   return res
  -- else
  --   print ("Valor de DATA inválido!")
  -- end if
end

function validarISO8601 (tipo, DATA)

  res = string.match(DATA, "(%d+-%d+-%d+)")
  return res
  -- if string.match(tipo, "DATA") and res= nil then
  --   return res
  -- else
  --   print ("Valor de DATA inválido!")
  -- end if
  
end