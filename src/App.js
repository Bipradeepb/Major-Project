
import React from 'react';
import NavBar from './components/Navbar';
import Hero from './components/Hero';
import DownloadSection from './components/DownloadSection';
import ProtocolSection from './components/ProtocolSection';
import Footer from './components/Footer';
import Features from './components/Features';
import { Box,Divider,Typography,useTheme} from '@mui/material';

function App() {

  const theme = useTheme();
  const isDarkMode = theme.palette.mode === 'dark';

  const apps = [
    { name: 'Hermes Selective Repeat', file: '/downloads/selective-repeat.deb', icon: '/downloads/linux.svg' },
    { name: 'Hermes Go-Back-N', file: '/downloads/go-back-n.deb', icon: '/downloads/linux.svg' },
    { name: 'Hermes Stop & Wait', file: '/downloads/stop-and-wait.deb', icon: '/downloads/linux.svg' },
  ];

  const protocols = [
    {
      key: 'selective-repeat',
      title: 'Selective Repeat',
      description: 'Selective Repeat is a sliding window ARQ protocol that allows individual acknowledgment of frames.',
      screenshots: ['/downloads/sr-log.png', '/downloads/sr-gui.png'],
      installCmd: ` 
1.Download .deb file
2.Installing the pkg (inside pkg_debian folder): sudo apt install ./selective-repeat.deb 
3.cd /opt directory
4.Refer Readme_user for complete steps
5.Removing the pkg (from anywhere): sudo apt purge selective-repeat.deb
  `,
    },
    {
      key: 'go-back-n',
      title: 'Go‑Back‑N',
      description: 'Go‑Back‑N ARQ lets the sender send multiple frames before needing an ACK, but on error re-sends a batch.',
      screenshots: ['/downloads/gbn-log.png', '/downloads/gbn-gui.png'],
      installCmd: ` 
1.Download .deb file
2.Installing the pkg (inside pkg_debian folder): sudo apt install ./go-back-n.deb
3.cd /opt directory
4.Refer Readme_user for complete steps
5.Removing the pkg (from anywhere): sudo apt purge go-back-n.deb
    `,
    },
    {
      key: 'stop-and-wait',
      title: 'Stop & Wait',
      description: 'Stop & Wait ARQ is the simplest, sending one frame and waiting for its ACK before next transmission.',
      screenshots: ['/downloads/sw-log.png', '/downloads/sw-gui.png'],
      installCmd: ` 
1.Download .deb file
2.Installing the pkg (inside pkg_debian folder): sudo apt install ./FTS.deb
3.cd /opt directory
4.Refer Readme_user for complete steps
5.Removing the pkg (from anywhere): sudo apt purge FTS.deb
    `,
    },
  ];

  return (
    <>
      <NavBar />
      <Hero />
      <Features />
      <Divider sx={{ my: 0 }} />
      <DownloadSection apps={apps} />
      <Divider sx={{ my: 0 }} />
      <Box sx={{ py: '30px', px: '30px', bgcolor: isDarkMode ? 'inherit' : '#FFF',mx:'auto' }}>
        <Box sx={{ maxWidth: 1200, mx: 'auto' }}>
          <Typography 
            variant="h4" 
            color="primary" 
            fontWeight="700" 
            sx={{ mb: 1 }}
          >
           Explore our products!
          </Typography>
        </Box>
      </Box>
      
      {protocols.map(p => (
        <ProtocolSection
          key={p.key}
          title={p.title}
          description={p.description}
          screenshots={p.screenshots}
          installCmd={p.installCmd}
        />
      ))}
      <Footer />
    </>
  );
}

export default App;