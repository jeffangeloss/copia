export interface LocalUser {
  id: string;
  email: string;
  name: string;
  role: "Operador" | "Administrador" | "Invitado";
  area: string;
  passwordHash: string;
}

function hashPassword(input: string): string {
  let hash = 0x811c9dc5;
  for (let i = 0; i < input.length; i++) {
    hash ^= input.charCodeAt(i);
    hash = Math.imul(hash, 0x01000193);
    hash >>>= 0;
  }
  return `fnv1a-${hash.toString(16).padStart(8, "0")}`;
}

const seededUsers: LocalUser[] = [
  {
    id: "adm-01",
    email: "gabylolicious@gmail.com",
    name: "Gabriela Garay",
    role: "Administrador",
    area: "Coordinación General",
    passwordHash: hashPassword("Gabriela12"),
  },
  {
    id: "adm-02",
    email: "jjjangelosss@gmail.com",
    name: "Jefferson Sanchez",
    role: "Administrador",
    area: "Coodinación Técnica",
    passwordHash: hashPassword("gabyjeff1617!!!"),
  },
  {
    id: "adm-03",
    email: "joaquin.castillo@gmail.com",
    name: "Joaquín Castillo",
    role: "Administrador",
    area: "Desarrollo de Software",
    passwordHash: hashPassword("Joaquin2025"),
  },
  {
    id: "adm-04",
    email: "andrea.barro@gmail.com",
    name: "Andrea Barro",
    role: "Administrador",
    area: "Arquitectura de Software",
    passwordHash: hashPassword("Admin#2025"),
  },
];

export function verifyLocalUser(
  email: string,
  password: string
): Omit<LocalUser, "passwordHash"> | null {
  const normalizedEmail = email.trim().toLowerCase();
  const passwordHash = hashPassword(password);
  const found = seededUsers.find((user) => user.email === normalizedEmail);
  if (!found) {
    return null;
  }
  if (found.passwordHash !== passwordHash) {
    return null;
  }
  const { passwordHash: _hash, ...safeUser } = found;
  return safeUser;
}

export function listLocalUsers(): Omit<LocalUser, "passwordHash">[] {
  return seededUsers.map(({ passwordHash, ...rest }) => rest);
}
